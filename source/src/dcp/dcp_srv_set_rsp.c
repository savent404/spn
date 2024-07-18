#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_set_rsp(struct dcp_ctx* ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_gen* block = PTR_OFFSET(hdr, sizeof(*hdr), struct dcp_block_gen);
  SPN_ASSERT("invalid length", length >= SPN_RTC_MINIMAL_FRAME_SIZE);
  hdr->service_id = DCP_SRV_ID_SET;
  hdr->service_type = ctx->ind_set_req_res ? DCP_SRV_TYPE_ERR : DCP_SRV_TYPE_RES;
  hdr->xid = SPN_HTONL(ctx->ind_xid);
  hdr->response_delay = SPN_HTONS(0);
  hdr->data_length = SPN_HTONS(sizeof(*block) + 4);
  block->option = DCP_OPTION_CONTROL;
  block->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
  block->length = SPN_HTONS(3);
  /* fill zero to the rest of buffer (minimal length is SPN_RTC_MINIMAL_FRAME_SIZE) */
  memset(PTR_OFFSET(block->data, 3, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - sizeof(*hdr) - sizeof(*block) - 4);
  *PTR_OFFSET(block->data, 0, uint8_t) = (ctx->ind_set_req_option >> 8) & 0xFF;
  *PTR_OFFSET(block->data, 1, uint8_t) = ctx->ind_set_req_option & 0xFF;
  *PTR_OFFSET(block->data, 2, uint8_t) = ctx->ind_set_req_res;
  return SPN_RTC_MINIMAL_FRAME_SIZE;
}
