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
  uint16_t dcp_length = SPN_NTOHS(hdr->data_length);
  uint16_t block_length;
  uint16_t exiface;
  db_value_t data;
  unsigned offset;
  int res;
  unsigned i;

  SPN_ASSERT("invalid inputs", ctx != NULL && mcs != NULL && payload != NULL && length > 0);

  if (length < dcp_length + sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid length\n");
    return -SPN_EBADMSG;
  }

  if (mcs->xid != dcp_get_xid(hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: xid missmatch, expected %04x, got %04x\n", mcs->xid,
                  dcp_get_xid(hdr));
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
        if (SPN_NTOHS(block->length) != 14) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }
        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 0, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_IP_BLOCK_INFO, 0, 0, 2, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        data.u32 = *(PTR_OFFSET(block->data, 2, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_ADDR, 0, 0, 4, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        data.u32 = *(PTR_OFFSET(block->data, 6, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_MASK, 0, 0, 4, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        data.u32 = *(PTR_OFFSET(block->data, 10, uint32_t));
        res = db_add_object(&interface.objects, DB_ID_IP_GATEWAY, 0, 0, 4, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS): {
        /**
         * @brief because there three are similar, deal these together
         */
        static const db_id_t ids[] = {DB_ID_NAME_OF_VENDOR, DB_ID_NAME_OF_INTERFACE, DB_ID_NAME_OF_INTERFACE};
        static const uint8_t sub_options[] = {DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,
                                              DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS};
        /* get block length */
        block_length = SPN_NTOHS(block->length);
        if (block_length < 2 || block_length > 255) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }

        /* Find idx */
        for (i = 0; i < ARRAY_SIZE(sub_options); i++) {
          if (sub_options[i] == block->sub_option) {
            break;
          }
        }
        SPN_ASSERT("Invalid sub_option", i < ARRAY_SIZE(sub_options));

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
        if (SPN_NTOHS(block->length) != 6) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }
        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_VENDOR_ID, 0, 0, 2, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);

        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 4, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_ID, 0, 0, 2, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE):
        if (SPN_NTOHS(block->length) != 4) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }
        data.u8 = *(PTR_OFFSET(block->data, 2, uint8_t));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_ROLE, 0, 0, 1, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE):
        if (SPN_NTOHS(block->length) != 4) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }
        data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
        res = db_add_object(&interface.objects, DB_ID_DEVICE_INSTANCE, 0, 0, 2, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS):
        block_length = SPN_NTOHS(block->length);
        data.u32 = 0;
        for (i = 2; i < block_length; i += 2) {
          uint8_t opt = *(PTR_OFFSET(block->data, i, uint8_t));
          uint8_t sub_opt = *(PTR_OFFSET(block->data, i + 1, uint8_t));
          data.u32 |= 1 << dcp_option_bit_idx(opt, sub_opt);
        }
        if (data.u32 == 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length, line %d\n", __LINE__);
          res = -SPN_EBADMSG;
          goto invalid_ret;
        }
        res = db_add_object(&interface.objects, DB_ID_DEVICE_OPTIONS, 0, 0, 4, &data);
        SPN_ASSERT("Add object failed", res == SPN_OK);
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
  SPN_ASSERT("Add interface failed", res == SPN_OK);

  res = db_get_interface(ctx->db, exiface, &intf);
  SPN_ASSERT("Get interface failed", res == SPN_OK);

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
