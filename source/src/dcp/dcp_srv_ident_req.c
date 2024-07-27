#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(h, l) ((h << 8) | l)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static void dcp_srv_ident_req_cb(void* arg) {
  struct dcp_mcs_ctx* mcs = (struct dcp_mcs_ctx*)arg;
  struct dcp_ctx* ctx = mcs->dcp_ctx;

  SPN_ASSERT("dcp_srv_ident_req_cb: invalid mcs", mcs != NULL);
  SPN_ASSERT("dcp_srv_ident_req_cb: invalid ctx", ctx != NULL);

  /* Devices are already added to db by dcp_srv_ident_cnf */
  memset(mcs, 0, sizeof(*mcs));
}

int dcp_srv_ident_req(struct dcp_ctx* ctx, struct dcp_mcs_ctx* mcs, struct pbuf* p) {
  int res = SPN_OK;
  unsigned idx, option, offset = 0;
  struct dcp_header* hdr;
  struct dcp_block_gen* block;
  uint32_t options = mcs->req_options_bitmap;
  const uint32_t required_opt = (1 << DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION) | (1 << DCP_BITMAP_ALL_SELECTOR) |
                                (1 << DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_ALIAS);

  /* Syntax check */
  if ((options & required_opt) == 0) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "dcp_srv_ident_req: missing required options\n");
    return -SPN_EINVAL;
  }

  if (mcs->state != DCP_STATE_IDLE) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "dcp_srv_ident_req: invalid state %d\n", mcs->state);
    return -SPN_EINVAL;
  }

  pbuf_remove_header(p, SPN_PDU_HDR_SIZE + sizeof(*hdr));

  for (idx = 0; idx < DCP_BITMAP_NUM && options; idx++) {
    if (!(options & (1 << idx))) {
      continue;
    }
    options &= ~(1 << idx);

    option = dcp_option_bit_offset(idx);
    block = PTR_OFFSET(p->payload, offset, struct dcp_block_gen);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "dcp_srv_ident_req: option %s(%02d:%02d)\n",
                  dcp_option_name(option >> 8, option & 0xFF), option >> 8, option & 0xFF);
    switch (option) {
      case BLOCK_TYPE(DCP_OPTION_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR):
        block->length = 0;
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        block->length = strlen(mcs->station_name);
        memcpy(block->data, mcs->station_name, block->length);
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS):
        block->length = strlen(mcs->alias_name);
        memcpy(block->data, mcs->alias_name, block->length);
        break;
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM):
        block->length = 12;
        *PTR_OFFSET(block->data, 0, uint32_t) = SPN_HTONL(mcs->ip_addr);
        *PTR_OFFSET(block->data, 4, uint32_t) = SPN_HTONL(mcs->ip_mask);
        *PTR_OFFSET(block->data, 8, uint32_t) = SPN_HTONL(mcs->ip_gw);
        break;
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
        block->length = 4;
        *PTR_OFFSET(block->data, 0, uint16_t) = SPN_HTONS(mcs->vendor_id);
        *PTR_OFFSET(block->data, 2, uint32_t) = SPN_HTONL(mcs->device_id);
        break;
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "dcp_srv_ident_req: unknown option %02d:%02d\n", option >> 8, option & 0xFF);
        goto fatal_err;
    }

    /* handle general block header */
    block->option = option >> 8;
    block->sub_option = option & 0xFF;
    offset += block->length + sizeof(*block);
    block->length = SPN_HTONS(block->length);
    offset = (offset + 1) & ~1; /* align to words */
  }

  if (offset < SPN_RTC_MINIMAL_FRAME_SIZE) {
    memset(PTR_OFFSET(p->payload, offset, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - offset);
    p->tot_len = SPN_RTC_MINIMAL_FRAME_SIZE;
  } else {
    p->tot_len = offset;
  }

  pbuf_add_header(p, sizeof(*hdr));
  hdr = (struct dcp_header*)p->payload;
  hdr->data_length = SPN_HTONS(offset);
  hdr->response_delay = SPN_HTONS(mcs->response_delay_factory);
  hdr->service_id = DCP_SRV_ID_IDENT;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  hdr->xid = SPN_HTONL(mcs->xid);

  pbuf_add_header(p, SPN_PDU_HDR_SIZE);
  *PTR_OFFSET(p->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_IDENT_REQ);

  /**
   * Response delay is in 10ms units, but the minimum value is 400ms.
   * 400ms if factor < 2
   * round(1s + 10ms * factor) if response_delay_factory >= 2
   *
   * @note Fuck the vendors, that giving factor(0) some meanings.
   */
  mcs->response_delay = mcs->response_delay_factory > 1 ? ((1999 + mcs->response_delay_factory * 10) / 1000) : 400;
  mcs->dcp_ctx = ctx;
  mcs->state = DCP_STATE_IDENT_REQ;
  mcs->dcp_ctx = ctx;

  sys_timeout(mcs->response_delay, dcp_srv_ident_req_cb, mcs);

  return SPN_OK;

fatal_err:
  return res;
}
