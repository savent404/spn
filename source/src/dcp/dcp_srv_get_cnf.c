#include <lwip/timeouts.h>
#include <spn/db_ll.h>
#include <spn/db_view.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/spn.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static inline void object_view_update_req(struct db_object* object) {
  /* TODO: indicate observer that value has been changed */
  SPN_UNUSED_ARG(object);
}

int dcp_srv_get_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs, void* payload, uint16_t length) {
  struct dcp_header* hdr;
  struct dcp_block_hdr* block;
  unsigned offset, dcp_length;

  SPN_UNUSED_ARG(length);

  hdr = PTR_OFFSET(payload, 0, struct dcp_header);
  dcp_length = SPN_NTOHS(hdr->data_length);
  if (SPN_NTOHL(hdr->xid) != ucs->xid) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: get.cnf: xid mismatch, expect %d, got %d\n", ucs->xid, SPN_NTOHL(hdr->xid));
    return SPN_EINVAL;
  }

  for (offset = sizeof(*hdr); offset < sizeof(*hdr) + dcp_length; offset += dcp_block_next(block)) {
    uint16_t option, block_length;
    struct db_object* object;
    int res;

    block = PTR_OFFSET(payload, offset, struct dcp_block_hdr);
    option = BLOCK_TYPE(block->option, block->sub_option);
    block_length = SPN_NTOHS(block->length);
    ucs->req_options_bitmap &= ~(1 << dcp_option_bitmap(block->option, block->sub_option));

    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: get.cnf: processing block %s(%04x)...\n",
                  dcp_option_name(option >> 8, option & 0xFF), option);

    switch (option) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM): {
        uint16_t block_info = SPN_NTOHS(*PTR_OFFSET(block->data, 0, uint16_t));
        uint32_t ip_addr = SPN_NTOHL(*PTR_OFFSET(block->data, 2, uint32_t));
        uint32_t ip_mask = SPN_NTOHL(*PTR_OFFSET(block->data, 6, uint32_t));
        uint32_t ip_gateway = SPN_NTOHL(*PTR_OFFSET(block->data, 10, uint32_t));
        SPN_ASSERT("invalid block length", block_length == 14);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_BLOCK_INFO, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u16 = block_info;
        object_view_update_req(object);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u32 = ip_addr;
        object_view_update_req(object);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u32 = ip_mask;
        object_view_update_req(object);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u32 = ip_gateway;
        object_view_update_req(object);
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION): {
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_INTERFACE, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        SPN_ASSERT("invalid block length", block_length >= 2);
        /* NOTE: more condition could save some cost of dynamic memory allocation
            but it's wired that we notice the name is changed somehow actually */
        db_free_objstr(object);
        db_dup_str2obj(object, PTR_OFFSET(block->data, 2, char), block_length - 2);
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID): {
        SPN_ASSERT("invalid block length", block_length == 6);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ID, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u16 = SPN_NTOHS(*PTR_OFFSET(block->data, 4, uint16_t));
        object_view_update_req(object);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_VENDOR_ID, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u16 = SPN_NTOHS(*PTR_OFFSET(block->data, 2, uint16_t));
        object_view_update_req(object);
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE): {
        SPN_ASSERT("invalid block length", block_length == 4);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ROLE, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        object->data.u16 = SPN_NTOHS(*PTR_OFFSET(block->data, 2, uint16_t));
        object_view_update_req(object);
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS): {
        SPN_ASSERT("so fucking boring!", 0);
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR): {
        SPN_ASSERT("invalid block length", block_length >= 2);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &object);
        SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
        db_free_objstr(object);
        db_dup_str2obj(object, PTR_OFFSET(block->data, 2, char), block_length - 2);
        break;
      }
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "Unhandled option:%04x\n", option);
        break;
    }
  }

  SPN_ASSERT("required options not answer yet!", !ucs->req_options_bitmap);

  ucs->state = DCP_STATE_IDLE;
  ucs->req_options_bitmap = 0;
  ucs->req_qualifier_bitmap = 0;
  ucs->xid++;
  sys_untimeout(_dcp_srv_get_req_timeout, ucs);
  return SPN_OK;
}
