#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_set_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_gen* block;
  int offset = sizeof(*hdr);

  SPN_UNUSED_ARG(length);

  /* Pack CONTROL::START */
  if (ucr_ctx->req_ctrl_start) {
    block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
    block->option = DCP_OPTION_CONTROL;
    block->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
    block->length = SPN_HTONS(3);
    *PTR_OFFSET(block->data, 0, uint8_t) = DCP_OPTION_CONTROL;
    *PTR_OFFSET(block->data, 1, uint8_t) = DCP_SUB_OPT_CTRL_START;
    *PTR_OFFSET(block->data, 2, uint8_t) = ucr_ctx->response;
    *PTR_OFFSET(block->data, 3, uint8_t) = 0; /* padding */
    offset += 8;
  }

  /* Pack CONTROL::RESPONSE */
  block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
  block->option = DCP_OPTION_CONTROL;
  block->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
  block->length = SPN_HTONS(3);
  *PTR_OFFSET(block->data, 0, uint8_t) = (ucr_ctx->req_option >> 8) & 0xFF;
  *PTR_OFFSET(block->data, 1, uint8_t) = ucr_ctx->req_option & 0xFF;
  *PTR_OFFSET(block->data, 2, uint8_t) = ucr_ctx->response;
  *PTR_OFFSET(block->data, 3, uint8_t) = 0;
  offset += 8;

  /* Pack CONTROL::STOP */
  if (ucr_ctx->req_ctrl_stop) {
    block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
    block->option = DCP_OPTION_CONTROL;
    block->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
    block->length = SPN_HTONS(3);
    *PTR_OFFSET(block->data, 0, uint8_t) = DCP_OPTION_CONTROL;
    *PTR_OFFSET(block->data, 1, uint8_t) = DCP_SUB_OPT_CTRL_STOP;
    *PTR_OFFSET(block->data, 2, uint8_t) = ucr_ctx->response;
    *PTR_OFFSET(block->data, 3, uint8_t) = 0;
    offset += 8;
  }

  hdr->service_id = DCP_SRV_ID_SET;
  hdr->service_type = ucr_ctx->response ? DCP_SRV_TYPE_ERR : DCP_SRV_TYPE_RES;
  hdr->xid = SPN_HTONL(ctx->ind_xid);
  hdr->response_delay = SPN_HTONS(0);
  hdr->data_length = SPN_HTONS(offset - sizeof(*hdr));

  /* fill zero to the rest of buffer (minimal length is SPN_RTC_MINIMAL_FRAME_SIZE) */
  SPN_ASSERT("payload is too big", offset < SPN_RTC_MINIMAL_FRAME_SIZE);
  memset(PTR_OFFSET(payload, offset - 1, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - offset);
  return SPN_RTC_MINIMAL_FRAME_SIZE;
}
