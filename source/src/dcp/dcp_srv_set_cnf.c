#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_set_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs, void* payload, uint16_t length) {
  struct dcp_header* hdr;
  struct dcp_block_gen* block;
  int res = SPN_OK;
  unsigned options = ucs->req_options_bitmap;
  uint16_t dcp_length, offset = sizeof(*hdr);

  SPN_UNUSED_ARG(ctx);

  hdr = (struct dcp_header*)payload;

  /* ignore it if xid didn't match */
  if (SPN_NTOHL(hdr->xid) != ucs->xid) {
    return -SPN_EAGAIN;
  }

  dcp_length = SPN_NTOHS(hdr->data_length);

  for (; offset < dcp_length; offset += dcp_block_next(block)) {
    uint16_t err;
    uint8_t opt, sub_opt;
    int idx;
    block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
    if (block->option != DCP_OPTION_CONTROL || block->sub_option != DCP_SUB_OPT_CTRL_RESPONSE) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid block, want a response block\n");
      continue;
    }
    if (block->length != 4) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid block length\n");
      continue;
    }
    opt = *PTR_OFFSET(block->data, 0, uint8_t);
    sub_opt = *PTR_OFFSET(block->data, 1, uint8_t);
    err = SPN_NTOHS(*PTR_OFFSET(block->data, 2, uint16_t));

    idx = dcp_option_bitmap(opt, sub_opt);
    if (idx < 0) {
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: invalid option %d.%d\n", opt, sub_opt);
      continue;
    }

    ucs->resp_errors[idx] = (enum dcp_block_error)err;
    options &= ~(1 << idx);

    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: response block %d.%d, err %d\n", opt, sub_opt, err);
  }
  if (options != 0) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.cnf: missing response blocks\n");
    return -SPN_EINVAL;
  }

  ucs->state = DCP_STATE_IDLE;
  ucs->xid++;
  sys_untimeout(_dcp_srv_set_req_timeout, ucs);

  return res;
}