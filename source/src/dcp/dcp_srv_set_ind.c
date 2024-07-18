#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static inline void obj_str_free(struct db_object* obj) {
  if (obj->header.is_dynamic) {
    free(obj->data.ptr);
  }
  obj->header.is_dynamic = 0;
  obj->header.len = 0;
}

static inline int obj_str_dup(struct db_object* obj, const char* str, unsigned len) {
  if (len < sizeof(obj->data.str)) {
    memcpy(obj->data.str, str, len);

    obj->header.is_dynamic = 0;
    obj->header.len = len;
  } else {
    obj->data.ptr = malloc(len);
    if (!obj->data.ptr) {
      return -SPN_ENOMEM;
    }
    memcpy(obj->data.ptr, str, len);
    obj->header.is_dynamic = 1;
    obj->header.len = len;
  }
  return SPN_OK;
}

int dcp_srv_set_ind(struct dcp_ctx* ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct db_object* obj;
  struct dcp_block_gen* block;
  uint16_t block_length, qualifier, dcp_length = SPN_NTOHS(hdr->data_length);
  uint16_t option, req_option;
  unsigned offset;
  int res = SPN_OK;

  SPN_UNUSED_ARG(length);

  for (offset = sizeof(*hdr); offset < sizeof(*hdr) + dcp_length; offset += dcp_block_next(block)) {
    block = PTR_OFFSET(hdr, offset, struct dcp_block_gen);
    qualifier = SPN_NTOHS(*PTR_OFFSET(block->data, 0, uint16_t));
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Handling block %s(%02x:%02x)\n",
                  dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
    option = BLOCK_TYPE(block->option, block->sub_option);
    switch (option) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM):
        SPN_ASSERT("invalid length", SPN_NTOHS(block->length) == 14);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get IP object\n");
          goto invalid_ret;
        }
        obj->data.u32 = SPN_NTOHL(*PTR_OFFSET(block->data, 2, uint32_t));
        db_object_updated_ind(ctx->db, obj, qualifier);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get subnet mask object\n");
          goto invalid_ret;
        }
        obj->data.u32 = SPN_NTOHL(*PTR_OFFSET(block->data, 6, uint32_t));
        db_object_updated_ind(ctx->db, obj, qualifier);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get gateway object\n");
          goto invalid_ret;
        }
        obj->data.u32 = SPN_NTOHL(*PTR_OFFSET(block->data, 10, uint32_t));
        db_object_updated_ind(ctx->db, obj, qualifier);
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        block_length = SPN_NTOHS(block->length);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_STATION, &obj);
        if (res < 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Failed to get station name object\n");
          goto invalid_ret;
        }
        /* TODO: Check name is lowercase, hard coded way */
        obj_str_free(obj);
        res = obj_str_dup(obj, PTR_OFFSET(block->data, 2, char), block_length);
        db_object_updated_ind(ctx->db, obj, qualifier);
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_START):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_STOP):
        /* TODO: indicate start/stop */
        /* NOTE: skip following code block, avoid override req_option */
        continue;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_FULL_SUITE):
      case BLOCK_TYPE(DCP_OPTION_DHCP, DCP_SUB_OPT_DHCP_UNKNOW):
      case BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN):
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP Set ind: Unsupported block %s(%02x:%02x)\n",
                      dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
        goto invalid_ret;
    }
    req_option = option;
  }

  ctx->ind_xid = SPN_NTOHL(hdr->xid);
  ctx->ind_delay_factory = SPN_NTOHS(hdr->response_delay);
  ctx->ind_set_req_option = req_option;
  ctx->ind_set_req_res = 0;
  return SPN_OK;
invalid_ret:
  ctx->ind_set_req_res = 1;
  return res;
}
