#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

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
  unsigned idx;

  if (length < sizeof(*hdr) + sizeof(*block)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: payload too short\n");
    return -SPN_EMSGSIZE;
  }

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
  mcr->state = DCP_STATE_IDENT_IND;

  /* Only one block in this request */
  switch (block->option << 8 | block->sub_option) {
    case DCP_OPTION_ALL_SELECTOR << 8 | DCP_SUB_OPT_ALL_SELECTOR:
      break;
    case DCP_OPTION_DEV_PROP << 8 | DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION:
      if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_STATION, &obj) != SPN_OK) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: get name of station failed\n");
        goto invalid_req;
      }
      if (dcp_obj_strncmp(obj, block->data, block->length) != 0) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: name of station mismatch\n");
        goto invalid_req;
      }
      break;
    case DCP_OPTION_DEV_PROP << 8 | DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR:
      if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &obj) != SPN_OK) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: get name of vendor failed\n");
        goto invalid_req;
      }
      if (dcp_obj_strncmp(obj, block->data, block->length) != 0) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: name of vendor mismatch\n");
        goto invalid_req;
      }
      break;
    default:
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: unknown option %d\n", block->option);
      goto invalid_req;
  }

  mcr->xid = SPN_NTOHL(hdr->xid);
  mcr->response_delay_factory = SPN_NTOHS(hdr->response_delay);
  mcr->response_delay = 0; /* TODO: need implement it */
  mcr->state = DCP_STATE_IDENT_RES;
  return SPN_OK;
invalid_req:
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_ind: invalid request, drop it...\n");
  mcr->state = DCP_STATE_IDLE;
  return -SPN_EAGAIN;
}
