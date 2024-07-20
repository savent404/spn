#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static inline int dcp_obj_strncmp(struct db_object* obj, const char* str, size_t len) {
  if (obj->header.is_dynamic) {
    return strncmp((char*)obj->data.ptr, str, len);
  } else {
    return strncmp(obj->data.str, str, len);
  }
}

int dcp_srv_ident_ind(struct dcp_ctx* ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_gen* block = (struct dcp_block_gen*)(hdr + 1);
  struct db_object* obj;
  struct dcp_mcr_ctx* mcr = NULL;
  uint32_t options = 0, offset = sizeof(*hdr);
  uint16_t option;
  uint16_t data_len;
  unsigned idx;

  if (length < SPN_NTOHS(hdr->data_length) + sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: payload too short\n");
    return -SPN_EMSGSIZE;
  }
  length = SPN_NTOHS(hdr->data_length) + sizeof(*hdr);

  /* Find empty MCR context */
  for (idx = 0; idx < ARRAY_SIZE(ctx->mcr_ctx); idx++) {
    if (ctx->mcr_ctx[idx].state == DCP_STATE_IDLE) {
      mcr = &ctx->mcr_ctx[idx];
      break;
    }
  }
  if (!mcr) {
    /* No free MCR context */
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: no free MCR context\n");
    return -SPN_EBUSY;
  }
  /* occupied MCR context from now on */
  mcr->state = DCP_STATE_IDENT_IND;

  for (; offset < length; offset += dcp_block_next(block)) {
    block = PTR_OFFSET(hdr, offset, struct dcp_block_gen);
    option = BLOCK_TYPE(block->option, block->sub_option);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: option %s(%02x:%02x)\n",
                  dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
    switch (option) {
      case BLOCK_TYPE(DCP_OPTION_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR):
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        data_len = SPN_NTOHS(block->length);
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_STATION, &obj) != SPN_OK) {
          SPN_ASSERT("You must have a name ok?", 0);
        }
        if (data_len != obj->header.len || dcp_obj_strncmp(obj, block->data, SPN_NTOHS(block->length)) != 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: name of station mismatch\n");
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
        data_len = SPN_NTOHS(block->length);
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &obj) != SPN_OK) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: get name of vendor failed\n");
          goto invalid_req;
        }
        if (data_len != obj->header.len || dcp_obj_strncmp(obj, block->data, SPN_NTOHS(block->length)) != 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: name of vendor mismatch\n");
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS):
        /* TODO: There are no needs to use name of alias */
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: unknown option %d\n", block->option);
        goto invalid_req;
    }
    options |= 1 << dcp_option_bitmap(block->option, block->sub_option);
  }

  if (offset != length) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: invalid block length\n");
    goto invalid_req;
  }

  /* First option check */
  if (offset == sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: no option found\n");
    goto invalid_req;
  }

  mcr->xid = SPN_NTOHL(hdr->xid);
  mcr->response_delay_factory = SPN_NTOHS(hdr->response_delay);
  mcr->response_delay = 0; /* TODO: need implement it */
  mcr->state = DCP_STATE_IDENT_RES;
  mcr->req_options_bitmap = options;
  return SPN_OK;
invalid_req:
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: invalid request, drop it...\n");
  mcr->state = DCP_STATE_IDLE;
  return -SPN_EAGAIN;
}
