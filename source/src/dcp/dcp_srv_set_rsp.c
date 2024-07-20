#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_set_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_gen* block;
  int offset = sizeof(*hdr), idx = 0;
  int has_ctrl_start = 0;
  int has_ctrl_stop = 0;

  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  /* pick start/stop firstly */
  if (ucr_ctx->req_options_bitmap & (1 << DCP_BITMAP_CONTROL_START)) {
    has_ctrl_start = 1;
  }
  if (ucr_ctx->req_options_bitmap & (1 << DCP_BITMAP_CONTROL_STOP)) {
    has_ctrl_stop = 1;
  }
  ucr_ctx->req_options_bitmap &= ~((1 << DCP_BITMAP_CONTROL_START) | (1 << DCP_BITMAP_CONTROL_STOP));

  /* Pack CONTROL::START */
  if (has_ctrl_start) {
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

  for (idx = 0; idx < DCP_BITMAP_NUM && ucr_ctx->req_options_bitmap; idx++) {
    if (ucr_ctx->req_options_bitmap & (1 << idx)) {
      uint16_t opt = dcp_option_bit_offset(idx);
      ucr_ctx->req_options_bitmap &= ~(1 << idx);
      block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
      block->option = DCP_OPTION_CONTROL;
      block->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
      block->length = SPN_HTONS(3);
      *PTR_OFFSET(block->data, 0, uint8_t) = (opt >> 8) & 0xFF;
      *PTR_OFFSET(block->data, 1, uint8_t) = opt & 0xFF;
      *PTR_OFFSET(block->data, 2, uint8_t) = ucr_ctx->response;
      *PTR_OFFSET(block->data, 3, uint8_t) = 0; /* fill padding, fuck that */
      offset += 8;
    }
  }

  /* Pack CONTROL::STOP */
  if (has_ctrl_stop) {
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
  hdr->xid = SPN_HTONL(ucr_ctx->xid);
  hdr->response_delay = SPN_HTONS(0);
  hdr->data_length = SPN_HTONS(offset - sizeof(*hdr));

  /* fill zero to the rest of buffer (minimal length is SPN_RTC_MINIMAL_FRAME_SIZE) */
  SPN_ASSERT("payload is too big", offset < SPN_RTC_MINIMAL_FRAME_SIZE);
  if (offset < SPN_RTC_MINIMAL_FRAME_SIZE) {
    memset(PTR_OFFSET(payload, offset - 1, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - offset);
  }
  return SPN_RTC_MINIMAL_FRAME_SIZE;
}
