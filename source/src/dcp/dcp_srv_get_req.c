#include <spn/db.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <spn/timeout.h>
#include <spn/lwip_spec.h>
#include <stdlib.h>
#include <string.h>

void _dcp_srv_get_req_timeout(void* arg) {
  struct dcp_ucs_ctx* ucs_ctx = (struct dcp_ucs_ctx*)arg;
  //@ assert \valid(ucs_ctx);
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
  //@ assert offset_hdr == SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  //@ assert offset_hdr == 12;

  /* TODO: check option is valid */
  offset = offset_hdr;
  options = ucs->req_options_bitmap;
  /*@
    @ loop invariant 0 <= idx <= DCP_BIT_IDX_NUM;
    @ loop invariant offset_hdr <= offset <= offset_hdr + 2 * DCP_BIT_IDX_NUM;
    @ loop invariant offset + 2 <= SPN_RTC_MINIMAL_FRAME_SIZE;
    @ loop assigns idx, offset, options, ((char*)payload)[offset_hdr .. offset_hdr + 2*DCP_BIT_IDX_NUM - 1];
    @ loop variant DCP_BIT_IDX_NUM - idx;
    @*/
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
    //@ assert offset + 2 <= SPN_RTC_MINIMAL_FRAME_SIZE;
    *PTR_OFFSET(payload, offset, uint16_t) = SPN_HTONS(option);
    offset += 2;
  }

  //@ assert offset >= offset_hdr;
  //@ assert offset <= offset_hdr + 2 * DCP_BIT_IDX_NUM;
  //@ assert SPN_PDU_HDR_SIZE + 2 <= SPN_RTC_MINIMAL_FRAME_SIZE;
  //@ assert offset <= SPN_RTC_MINIMAL_FRAME_SIZE;

  hdr = PTR_OFFSET(payload, SPN_PDU_HDR_SIZE, struct dcp_header);
  //@ assert \valid(hdr);
  //@ assert \valid(&hdr->service_id);
  hdr->service_id = DCP_SRV_ID_GET;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  dcp_set_xid(hdr, ucs->xid);
  hdr->response_delay = 0;
  hdr->data_length = SPN_HTONS(offset - offset_hdr);

  //@ assert \valid((uint16_t*)payload);
  *PTR_OFFSET(payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);

  /* update output length */
  //@ assert offset <= SPN_RTC_MINIMAL_FRAME_SIZE;
  *length = dcp_padding(payload, offset);
  //@ assert *length >= offset;
  //@ assert *length <= SPN_RTC_MINIMAL_FRAME_SIZE;

  ucs->state = DCP_STATE_GET_REQ;

  SPN_TIMEOUT(SPN_DCP_UC_TIMEOUT, _dcp_srv_get_req_timeout, ucs);
  return SPN_OK;
}
