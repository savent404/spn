#include <spn/db.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <spn/timeout.h>
#include <stdlib.h>
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

int dcp_srv_get_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs, void* payload, uint16_t* length) {
  struct dcp_header* hdr = NULL;
  unsigned offset, idx, options;
  const unsigned offset_hdr = SPN_PDU_HDR_SIZE + sizeof(*hdr);

  SPN_UNUSED_ARG(ctx);

  SPN_ASSERT("Invalid external interface id", ucs->ex_ifr >= SPN_EXTERNAL_INTERFACE_BASE);

  /* TODO: check option is valid */
  offset = offset_hdr;
  options = ucs->req_options_bitmap;
  for (idx = 0; idx < DCP_BIT_IDX_NUM && options; idx++) {
    uint16_t option;
    if ((options & (1 << idx)) == 0) {
      continue;
    }
    /* clear the bit */
    options &= ~(1 << idx);

    option = dcp_option_from_bit_idx(idx);
    SPN_ASSERT("invalid option", option);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: get.req: quering type: %s(%04x)...\n",
                  dcp_option_name(option >> 8, option & 0xFF), option);
    *PTR_OFFSET(payload, offset, uint16_t) = SPN_HTONS(option);
    offset += 2;
  }

  hdr = PTR_OFFSET(payload, SPN_PDU_HDR_SIZE, struct dcp_header);
  hdr->service_id = DCP_SRV_ID_GET;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  dcp_set_xid(hdr, ucs->xid);
  hdr->response_delay = 0;
  hdr->data_length = SPN_HTONS(offset - offset_hdr);

  *PTR_OFFSET(payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);

  /* update output length */
  *length = dcp_padding(payload, offset);

  ucs->state = DCP_STATE_GET_REQ;

  SPN_TIMEOUT(SPN_DCP_UC_TIMEOUT, _dcp_srv_get_req_timeout, ucs);
  return SPN_OK;
}
