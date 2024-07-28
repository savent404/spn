#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option) << 8 | (sub_option))
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_ident_cnf(struct dcp_ctx* ctx,
                      struct dcp_mcs_ctx* mcs,
                      void* payload,
                      uint16_t length,
                      uint16_t* pexiface) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_hdr* block;
  struct db_interface interface, *intf;
  struct db_object* obj;
  uint16_t dcp_length = SPN_NTOHS(hdr->data_length), block_length, exiface;
  db_value_t data;
  unsigned offset;
  int res;
  unsigned i;

  if (length < dcp_length + sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid length\n");
    return -SPN_EBADMSG;
  }

  if (mcs->xid != dcp_get_xid(hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid xid\n");
    return -SPN_ENXIO;
  }

  /* Reset temp interface */
  memset(&interface, 0, sizeof(interface));

  for (offset = sizeof(*hdr); offset < sizeof(*hdr) + dcp_length; offset += dcp_block_next(block)) {
    block = PTR_OFFSET(payload, offset, struct dcp_block_hdr);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: handling block %s(%02x:%02x)\n",
                  dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
    if (offset > length) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length\n");
      res = -SPN_EBADMSG;
      goto invalid_ret;
    }
    switch (BLOCK_TYPE(block->option, block->sub_option)) {
      case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
        SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 14);

        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 0, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_IP_BLOCK_INFO, 0, 0, 2, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }

        data.u32 = *(PTR_OFFSET(block->data, 2, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_ADDR, 0, 0, 4, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }

        data.u32 = *(PTR_OFFSET(block->data, 6, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_MASK, 0, 0, 4, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }

        data.u32 = *(PTR_OFFSET(block->data, 10, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_GATEWAY, 0, 0, 4, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS): {
        static const db_id_t ids[] = {DB_ID_NAME_OF_VENDOR, DB_ID_NAME_OF_INTERFACE, DB_ID_NAME_OF_INTERFACE};
        static const uint8_t sub_options[] = {DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,
                                              DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS};
        block_length = SPN_NTOHS(block->length);
        SPN_ASSERT("Invalid block length", block_length > 2 && block_length < 256);

        /* Find idx */
        for (i = 0; i < ARRAY_SIZE(sub_options); i++) {
          if (sub_options[i] == block->sub_option) {
            break;
          }
        }
        if (i == ARRAY_SIZE(sub_options)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: unhandled option %04x\n",
                        BLOCK_TYPE(block->option, block->sub_option));
          res = -SPN_EINVAL;
          goto invalid_ret;
        }
        /* For re-use db_dup_str2obj, add object firstly */
        res = db_add_object(&interface.objects, ids[i], 0, 0, 0, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        res = db_get_object(&interface.objects, ids[i], &obj);
        SPN_ASSERT("Get object failed", res == SPN_OK);
        res = db_dup_str2obj(obj, PTR_OFFSET(block->data, 2, const char), block_length - 2);
        SPN_ASSERT("db_dup_str2obj failed", res == SPN_OK);
        break;
      }
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
        SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 6);
        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_VENDOR_ID, 0, 0, 2, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }

        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 4, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_ID, 0, 0, 2, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE):
        SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 4);
        data.u8 = *(PTR_OFFSET(block->data, 2, uint8_t));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_ROLE, 0, 0, 1, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE):
        SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 4);
        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_INSTANCE, 0, 0, 2, &data);
        if (res != SPN_OK) {
          goto invalid_ret;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS):
        block_length = SPN_NTOHS(block->length);
        data.u32 = 0;
        for (i = 2; i < block_length; i += 2) {
          uint8_t opt = *(PTR_OFFSET(block->data, i, uint8_t));
          uint8_t sub_opt = *(PTR_OFFSET(block->data, i + 1, uint8_t));
          data.u32 |= 1 << dcp_option_bit_idx(opt, sub_opt);
        }
        res = db_add_object(&interface.objects, DB_ID_DEVICE_OPTIONS, 0, 0, 4, &data);
        if (res != SPN_OK) {
          free(data.ptr);
          goto invalid_ret;
        }
        break;
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: unhandled option %04x\n",
                      BLOCK_TYPE(block->option, block->sub_option));
        break;
    }
  }

  /* Find empty interface and assigned new interface_id */
  /* FIXME: its not thread-safe */
  exiface = mcs->external_interface_id++;
  res = db_add_interface(ctx->db, exiface);
  if (res < 0) {
    goto invalid_ret;
  }

  res = db_get_interface(ctx->db, exiface, &intf);
  if (res < 0 || !intf) {
    goto invalid_ret;
  }

  res = db_dup_interface(intf, &interface);
  if (res != SPN_OK) {
    res = -SPN_ENOMEM;
    goto cleanup_interface;
  }

  if (pexiface) {
    *pexiface = exiface;
  }
  db_del_interface(&interface);
  return SPN_OK;
cleanup_interface:
  mcs->external_interface_id--; /* FIXME: its not thread-safe */
  db_del_interface(intf);
invalid_ret:
  db_del_interface(&interface);
  return res;
}
