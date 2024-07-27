#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

void _dcp_srv_get_req_timeout(void* arg) {
  struct dcp_ucs_ctx* ucs_ctx = (struct dcp_ucs_ctx*)arg;
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: get.req timeout!\n");

  ucs_ctx->state = DCP_STATE_IDLE;
  ucs_ctx->req_options_bitmap = 0;
  ucs_ctx->req_qualifier_bitmap = 0;
  ucs_ctx->xid++;
}

int dcp_srv_get_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs, struct pbuf* p) {
  struct dcp_header* hdr = NULL;
  unsigned offset, idx, options;

  SPN_UNUSED_ARG(ctx);

  /* TODO: check option is valid */

  pbuf_remove_header(p, sizeof(*hdr) + SPN_PDU_HDR_SIZE);

  offset = 0;
  options = ucs->req_options_bitmap;
  for (idx = 0; idx < DCP_BITMAP_NUM && options; idx++) {
    uint16_t option;
    if ((options & (1 << idx)) == 0) {
      continue;
    }
    /* clear the bit */
    options &= ~(1 << idx);

    option = dcp_option_bit_offset(idx);
    SPN_ASSERT("invalid option", option);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: get.req: quering type: %s(%04x)...\n",
                  dcp_option_name(option >> 8, option & 0xFF), option);
    *PTR_OFFSET(p->payload, offset, uint16_t) = SPN_HTONS(option);
    offset += 2;
  }

  pbuf_add_header(p, sizeof(*hdr));
  hdr = (struct dcp_header*)p->payload;
  hdr->service_id = DCP_SRV_ID_GET;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  hdr->xid = SPN_HTONS(ucs->xid);
  hdr->response_delay = 0;
  hdr->data_length = SPN_HTONS(offset);

  /* update output length */
  if (offset + sizeof(*hdr) < SPN_DCP_MIN_SIZE) {
    memcpy(PTR_OFFSET(p->payload, offset, uint8_t), 0, SPN_DCP_MIN_SIZE - offset - sizeof(*hdr));
    p->tot_len = SPN_DCP_MIN_SIZE;
  } else {
    p->tot_len = offset + sizeof(*hdr);
  }

  pbuf_add_header(p, SPN_PDU_HDR_SIZE);
  *PTR_OFFSET(p->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);

  ucs->state = DCP_STATE_GET_REQ;

  sys_timeout(SPN_DCP_UC_TIMEOUT, _dcp_srv_get_req_timeout, ucs);
  return SPN_OK;
}
