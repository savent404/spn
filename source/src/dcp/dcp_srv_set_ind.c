#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static inline int has_upper_case(const char* str, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (str[i] >= 'A' && str[i] <= 'Z') {
      return 1;
    }
  }
  return 0;
}

int dcp_srv_set_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct db_object* obj;
  struct dcp_block_gen* block;
  uint16_t block_length, dcp_length = SPN_NTOHS(hdr->data_length);
  uint32_t req_options = 0, qualifier;
  unsigned offset, bitmap_idx;
  int res;

  SPN_UNUSED_ARG(length);

  for (offset = sizeof(*hdr); offset < sizeof(*hdr) + dcp_length; offset += dcp_block_next(block)) {
    enum dcp_block_error err = DCP_BLOCK_ERR_OK;
    block = PTR_OFFSET(hdr, offset, struct dcp_block_gen);
    qualifier = SPN_NTOHS(*PTR_OFFSET(block->data, 0, uint16_t));
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Handling block %s(%02x:%02x)\n",
                  dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);

    /* general attribute needed for all blocks */
    bitmap_idx = dcp_option_bitmap(block->option, block->sub_option);
    req_options |= 1 << bitmap_idx;

    /* TODO: Global check, if we are in operational state, reject all */

    switch (BLOCK_TYPE(block->option, block->sub_option)) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM):
        SPN_ASSERT("invalid length", SPN_NTOHS(block->length) == 14);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get IP object\n");
          err = DCP_BLOCK_ERR_LOCAL_ERR;
          goto internal_err;
        }
        obj->data.u32 = *PTR_OFFSET(block->data, 2, uint32_t);
        db_object_updated_ind(ctx->db, obj, qualifier);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get subnet mask object\n");
          err = DCP_BLOCK_ERR_LOCAL_ERR;
          goto internal_err;
        }
        obj->data.u32 = *PTR_OFFSET(block->data, 6, uint32_t);
        db_object_updated_ind(ctx->db, obj, qualifier);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get gateway object\n");
          goto internal_err;
        }
        obj->data.u32 = *PTR_OFFSET(block->data, 10, uint32_t);
        db_object_updated_ind(ctx->db, obj, qualifier);
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        block_length = SPN_NTOHS(block->length);
        if (has_upper_case(PTR_OFFSET(block->data, 2, char), block_length - 2)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Station name contains upper case characters\n");
          err = DCP_BLOCK_ERR_RESOURCE_ERR;
          break;
        }
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_INTERFACE, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get station name object\n");
          goto internal_err;
        }
        db_free_objstr(obj);
        res = db_dup_str2obj(obj, PTR_OFFSET(block->data, 2, char), block_length - 2);
        db_object_updated_ind(ctx->db, obj, qualifier);
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_START):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_STOP):
        /* TODO: indicate start/stop */
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_FULL_SUITE):
      case BLOCK_TYPE(DCP_OPTION_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT):
      case BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN):
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Unsupported block %s(%02x:%02x)\n",
                      dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
        err = DCP_BLOCK_ERR_OPTION_NOT_SUPPORTED;
        break;
    }
    ucr_ctx->error[bitmap_idx] = err;
    continue;
  internal_err: /* NOTE: for page saving, set error code in public section */
    err = DCP_BLOCK_ERR_LOCAL_ERR;
    ucr_ctx->error[bitmap_idx] = err;
  }

  ucr_ctx->xid = SPN_NTOHL(hdr->xid);
  ucr_ctx->req_options_bitmap = req_options;
  return SPN_OK;
}
