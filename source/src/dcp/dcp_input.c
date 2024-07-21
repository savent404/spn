#include <netif/ethernet.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))
#define BLOCK_TYPE(h, l) ((h << 8) | l)

static void dcp_mcr_rsp_callback(void* arg) {
  struct dcp_mcr_ctx* mcr_ctx = (struct dcp_mcr_ctx*)arg;
  struct dcp_ctx* ctx = mcr_ctx->dcp_ctx;
  struct pbuf* out;
  int res;

  out = pbuf_alloc(PBUF_RAW, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
  SPN_ASSERT("pbuf_alloc failed", out != NULL);

  pbuf_header(out, SPN_PDU_HDR_SIZE);

  res = dcp_srv_ident_rsp(ctx, mcr_ctx, out->payload, out->tot_len);
  SPN_ASSERT("dcp_srv_ident_rsp failed", res <= 0);
  out->tot_len = res;

  pbuf_header(out, -SPN_PDU_HDR_SIZE);
  *PTR_OFFSET(out->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_IDENT_RES);

  ethernet_output(NULL, out, NULL, &mcr_ctx->src_addr, ETHTYPE_PROFINET);
}

int dcp_input(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* src, struct pbuf* rtc_pdu) {
  uint16_t frame_id;
  struct dcp_header* hdr;
  struct pbuf* out;
  int res;
  unsigned idx;

  SPN_UNUSED_ARG(iface);

  frame_id = SPN_NTOHS(*PTR_OFFSET(rtc_pdu->payload, 0, uint16_t));
  hdr = PTR_OFFSET(rtc_pdu->payload, 2, struct dcp_header);

  switch (frame_id) {
    case FRAME_ID_DCP_HELLO_REQ:
    case FRAME_ID_DCP_IDENT_RES:
      /* TODO: This is for IOC use */
      return -SPN_ENOSYS;
    case FRAME_ID_DCP_GET_SET:
      /* TODO: if in operating mode, should ignore this request */
      if (hdr->service_id == DCP_SRV_ID_GET && hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;
        res = dcp_srv_get_ind(ctx, &ucr, hdr, rtc_pdu->tot_len - 2);
        SPN_ASSERT("dcp_srv_get_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_RAW, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        pbuf_header(out, SPN_PDU_HDR_SIZE);

        res = dcp_srv_get_rsp(ctx, &ucr, out->payload, out->tot_len);
        SPN_ASSERT("dcp_srv_get_rsp failed", res <= 0);
        out->tot_len = res;

        pbuf_header(out, -SPN_PDU_HDR_SIZE);
        *PTR_OFFSET(out->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);
      } else if (hdr->service_id == DCP_SRV_ID_SET && hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;

        res = dcp_srv_set_ind(ctx, &ucr, hdr, rtc_pdu->tot_len - 2);
        SPN_ASSERT("dcp_srv_set_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_RAW, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        pbuf_header(out, SPN_PDU_HDR_SIZE);

        res = dcp_srv_set_rsp(ctx, &ucr, out->payload, out->tot_len);
        SPN_ASSERT("dcp_srv_set_rsp failed", res <= 0);
        out->tot_len = res;

        pbuf_header(out, -SPN_PDU_HDR_SIZE);
        *PTR_OFFSET(out->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);
      } else if (hdr->service_id == DCP_SRV_ID_SET && hdr->service_type == DCP_SRV_TYPE_RES) {
        /* TODO: implement this */
        return -SPN_ENOSYS;
      } else if (hdr->service_id == DCP_SRV_ID_GET && hdr->service_type == DCP_SRV_TYPE_RES) {
        /* TODO: implement this */
        return -SPN_ENOSYS;
      } else {
        return -SPN_EINVAL;
      }

      if (out) {
        ethernet_output(NULL, out, NULL, src, ETHTYPE_PROFINET);
      }
      break;
    case FRAME_ID_DCP_IDENT_REQ:
      res = SPN_ENOBUFS;
      for (idx = 0; idx < ARRAY_SIZE(ctx->mcr_ctx); idx++) {
        if (ctx->mcr_ctx[idx].state == DCP_STATE_IDLE) {
          res = dcp_srv_ident_ind(ctx, &ctx->mcr_ctx[idx], hdr, rtc_pdu->tot_len - 2);
          break;
        }
      }

      if (res != SPN_OK) {
        return res;
      }

      memcpy(&ctx->mcr_ctx[idx].src_addr, src, sizeof(struct eth_addr));
      if (ctx->mcr_ctx[idx].response_delay == 0) {
        dcp_mcr_rsp_callback(&ctx->mcr_ctx[idx]);
      } else {
        /* TODO: trigger a timer to call dcp_mcr_rsp_callback */
        return -SPN_ENOSYS;
      }
      break;
    default:
      return -SPN_EINVAL;
  }
  return SPN_OK;
}
