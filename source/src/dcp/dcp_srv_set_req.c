#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

void _dcp_srv_set_req_timeout(void* arg) {
  struct dcp_ucs_ctx* ucs_ctx = (struct dcp_ucs_ctx*)arg;
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.req timeout!\n");

  ucs_ctx->state = DCP_STATE_IDLE;
  ucs_ctx->req_options_bitmap = 0;
  ucs_ctx->req_qualifier_bitmap = 0;
}

int dcp_srv_set_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, struct pbuf* p) {
  struct dcp_header* hdr = (struct dcp_header*)p->payload;
  unsigned offset = 0;
  int res = SPN_OK;
  unsigned idx;
  unsigned options = ucs_ctx->req_options_bitmap, qualifer = ucs_ctx->req_qualifier_bitmap;

  pbuf_remove_header(p, sizeof(*hdr));

  for (idx = 0; idx < DCP_BITMAP_NUM && options; idx++) {
    const uint16_t type = dcp_option_bit_offset(idx);
    uint16_t qual = qualifer & (1 << idx) ? SPN_HTONS(DCP_QUALIFER_PERSISTENT) : SPN_HTONS(DCP_QUALIFER_TEMP);
    struct dcp_block_gen* block = PTR_OFFSET(hdr, offset, struct dcp_block_gen);
    options &= ~(1 << idx);

    switch (type) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM):
        block->length = 14;
        *PTR_OFFSET(block->data, 2, uint32_t) = ucs_ctx->ip_addr;
        *PTR_OFFSET(block->data, 6, uint32_t) = ucs_ctx->ip_mask;
        *PTR_OFFSET(block->data, 10, uint32_t) = ucs_ctx->ip_gw;
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        SPN_ASSERT("station name is NULL", ucs_ctx->station_name);
        block->length = 2 + strlen(ucs_ctx->station_name);
        memcpy(PTR_OFFSET(block->data, 2, char), ucs_ctx->station_name, block->length - 2);
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_START):
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_STOP):
        block->length = 2;
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
        block->length = 4;
        *PTR_OFFSET(block->data, 2, uint16_t) = SPN_PP_HTONS(DCP_SIGNAL_LED_FLASH);
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
        block->length = 2;
        qual = ucs_ctx->factory_reset_qualifer;
        break;
      case BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
        block->length = 2;
        qual = ucs_ctx->reset_to_factory_qualifer;
        break;
    }

    /* first 2 bytes of payload always QualiferBlock */
    block->option = type >> 8;
    block->sub_option = type & 0xff;
    *PTR_OFFSET(block->data, 0, uint16_t) = qual;

    /* set padding to zero if length is odd */
    if (block->length & 1) {
      *PTR_OFFSET(block->data, block->length, uint8_t) = 0;
      block->length++;
    }

    /* Swap length after calculated offset */
    offset += sizeof(*block) + block->length;
    block->length = SPN_HTONS(block->length);
  }

  pbuf_add_header(p, sizeof(*hdr));

  if ((offset + sizeof(*hdr)) < SPN_DCP_MIN_SIZE) {
    memset(PTR_OFFSET(hdr, offset + sizeof(*hdr), uint8_t), 0, SPN_DCP_MIN_SIZE - offset - sizeof(*hdr));
    p->tot_len = SPN_DCP_MIN_SIZE;
  } else {
    p->tot_len = offset + sizeof(*hdr);
  }

  hdr->service_id = DCP_SRV_ID_SET;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  hdr->data_length = SPN_NTOHS(offset);
  hdr->response_delay = 0; /* NOTE: this is reserved, need to be zero */
  hdr->xid = SPN_NTOHL(hdr->xid++);

  sys_timeout(SPN_DCP_UC_TIMEOUT, _dcp_srv_set_req_timeout, ucs_ctx);

  return res;
}
