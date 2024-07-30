#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/timeout.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static void set_cnf_db_update(struct db_ctx* db, struct dcp_ucs_ctx* ucs, uint16_t options) {
  int res;
  struct db_object* obj;
  switch (options) {
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
      res = db_get_interface_object(db, ucs->ex_ifr, DB_ID_NAME_OF_INTERFACE, &obj);
      SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
      db_free_objstr(obj);
      db_dup_str2obj(obj, ucs->station_name, strlen(ucs->station_name));
      db_object_updated_ind(db, obj, 0);
      break;
    case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
      res = db_get_interface_object(db, ucs->ex_ifr, DB_ID_IP_ADDR, &obj);
      SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
      obj->data.u32 = ucs->ip_addr;
      db_object_updated_ind(db, obj, 0);

      res = db_get_interface_object(db, ucs->ex_ifr, DB_ID_IP_MASK, &obj);
      SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
      obj->data.u32 = ucs->ip_mask;
      db_object_updated_ind(db, obj, 0);

      res = db_get_interface_object(db, ucs->ex_ifr, DB_ID_IP_GATEWAY, &obj);
      SPN_ASSERT("db_get_interface_object failed", res == SPN_OK);
      obj->data.u32 = ucs->ip_gw;
      db_object_updated_ind(db, obj, 0);
      break;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START):
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP):
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
      break;
    default:
      SPN_ASSERT("unknown option, how to update?", 0);
  }
}

int dcp_srv_set_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs, void* payload, uint16_t length) {
  struct dcp_header* hdr;
  struct dcp_block_hdr* block;
  int res = SPN_OK;
  unsigned options = ucs->req_options_bitmap;
  uint16_t dcp_length, offset = sizeof(*hdr);

  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  SPN_ASSERT("Invalid external interface id", ucs->ex_ifr >= SPN_DB_MAX_INTERFACE);

  hdr = (struct dcp_header*)payload;

  /* ignore it if xid didn't match */
  if (dcp_get_xid(hdr) != ucs->xid) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: xid mismatched expect %d, got %d\n", ucs->xid, dcp_get_xid(hdr));
    return -SPN_EAGAIN;
  }

  dcp_length = SPN_NTOHS(hdr->data_length);

  for (; offset < dcp_length + sizeof(*hdr); offset += dcp_block_next(block)) {
    uint8_t err;
    uint8_t opt, sub_opt;
    uint16_t block_length;
    int idx;
    block = PTR_OFFSET(payload, offset, struct dcp_block_hdr);
    block_length = SPN_NTOHS(block->length);
    if (block->option != DCP_OPT_CONTROL || block->sub_option != DCP_SUB_OPT_CTRL_RESPONSE) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid block, want a response block\n");
      continue;
    }
    if (block_length != 3) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid block length\n");
      continue;
    }
    opt = *PTR_OFFSET(block->data, 0, uint8_t);
    sub_opt = *PTR_OFFSET(block->data, 1, uint8_t);
    err = *PTR_OFFSET(block->data, 2, uint8_t);

    idx = dcp_option_bit_idx(opt, sub_opt);
    if (idx < 0) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid option %d.%d\n", opt, sub_opt);
      continue;
    }

    ucs->resp_errors[idx] = (enum dcp_block_error)err;
    options &= ~(1 << idx);

    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: response block %d.%d, err %d\n", opt, sub_opt, err);

    if (err == DCP_BLOCK_ERR_OK) {
      set_cnf_db_update(ctx->db, ucs, BLOCK_TYPE(opt, sub_opt));
    }
  }
  if (options != 0) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: missing response blocks\n");
    return -SPN_EINVAL;
  }

  ucs->state = DCP_STATE_IDLE;
  ucs->xid++;
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: done, channel timeout...\n");
  SPN_UNTIMEOUT(_dcp_srv_set_req_timeout, ucs);

  return res;
}