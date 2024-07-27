#include <lwip/timeouts.h>
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
  struct db_interface* db_interface;
  struct db_port* db_port;
  struct db_object* obj;
  struct pbuf* out;
  int res;
  unsigned idx;

  res = db_get_interface(ctx->db, ctx->interface_id, &db_interface);
  SPN_ASSERT("db_get_interface failed", res == SPN_OK);

  out = pbuf_alloc(PBUF_LINK, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
  SPN_ASSERT("pbuf_alloc failed", out != NULL);

  pbuf_remove_header(out, SPN_PDU_HDR_SIZE);
  res = dcp_srv_ident_rsp(ctx, mcr_ctx, out->payload, out->tot_len);
  SPN_ASSERT("dcp_srv_ident_rsp failed", res > 0);
  out->tot_len = res;
  pbuf_add_header(out, SPN_PDU_HDR_SIZE);

  *PTR_OFFSET(out->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_IDENT_RES);

  /* TODO: chose iface by LLDP if is unicast frame */
  if (1 /*is_multicast_addr(&mcr_ctx->src_addr) */) {
    for (idx = 0; idx < ARRAY_SIZE(db_interface->ports); idx++) {
      res = db_get_port(db_interface, idx, &db_port);
      if (res != SPN_OK) {
        continue;
      }

      res = db_get_object(&(db_port->objects), DB_ID_IFACE, &obj);
      SPN_ASSERT("db_get_object failed", res == SPN_OK);

      res = dcp_output(ctx, (struct spn_iface*)obj->data.ptr, &mcr_ctx->src_addr, out);
      SPN_ASSERT("dcp_output failed", res == SPN_OK);
    }
  }

  pbuf_free(out);
}

int dcp_input(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* src, struct pbuf* rtc_pdu) {
  uint16_t frame_id;
  struct dcp_header* hdr;
  struct pbuf* out = NULL;
  uint16_t ex_iface;
  struct db_interface* db_iface;
  struct db_object* db_obj;
  int res;
  unsigned idx;

  SPN_UNUSED_ARG(iface);

  frame_id = SPN_NTOHS(*PTR_OFFSET(rtc_pdu->payload, 0, uint16_t));
  hdr = PTR_OFFSET(rtc_pdu->payload, 2, struct dcp_header);

  switch (frame_id) {
    case FRAME_ID_DCP_HELLO_REQ:
    case FRAME_ID_DCP_IDENT_RES:
      if (hdr->service_id != DCP_SRV_ID_IDENT && hdr->service_type != DCP_SRV_TYPE_RES) {
        return -SPN_EINVAL;
      }
      /**
       * @brief call ident.cnf and fill the mac address info if device did not response it
       */
      res = dcp_srv_ident_cnf(ctx, &ctx->mcs_ctx, hdr, rtc_pdu->tot_len - 2, &ex_iface);
      SPN_ASSERT("dcp_srv_ident_cnf failed", res == SPN_OK);

      if (db_get_interface_object(ctx->db, ex_iface, DB_ID_IP_MAC_ADDR, &db_obj) == -SPN_ENOENT) {
        db_value_t val;
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "device didn't response mac address, fill it based on ethframe\n");
        pbuf_add_header(rtc_pdu, SPN_PDU_HDR_SIZE);
        memcpy(val.mac, PTR_OFFSET(rtc_pdu->payload, 6, uint8_t), sizeof(val.mac));
        pbuf_remove_header(rtc_pdu, SPN_PDU_HDR_SIZE);

        res = db_get_interface(ctx->db, ex_iface, &db_iface);
        SPN_ASSERT("db_get_interface failed", res == SPN_OK);
        res = db_add_object(&db_iface->objects, DB_ID_IP_MAC_ADDR, 0, 0, sizeof(val.mac), &val);
        SPN_ASSERT("db_add_object failed", res == SPN_OK);
      }
      break;
    case FRAME_ID_DCP_GET_SET:
      /* TODO: if in operating mode, should ignore this request */
      if (hdr->service_id == DCP_SRV_ID_GET && hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;
        res = dcp_srv_get_ind(ctx, &ucr, hdr, rtc_pdu->tot_len - 2);
        SPN_ASSERT("dcp_srv_get_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_LINK, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        pbuf_remove_header(out, SPN_PDU_HDR_SIZE);
        res = dcp_srv_get_rsp(ctx, &ucr, out->payload, out->tot_len);
        SPN_ASSERT("dcp_srv_get_rsp failed", res <= 0);
        out->tot_len = res;
        pbuf_add_header(out, -SPN_PDU_HDR_SIZE);

      } else if (hdr->service_id == DCP_SRV_ID_SET && hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;

        res = dcp_srv_set_ind(ctx, &ucr, hdr, rtc_pdu->tot_len - 2);
        SPN_ASSERT("dcp_srv_set_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_LINK, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        pbuf_remove_header(out, SPN_PDU_HDR_SIZE);
        res = dcp_srv_set_rsp(ctx, &ucr, out->payload, out->tot_len);
        SPN_ASSERT("dcp_srv_set_rsp failed", res > 0);
        out->tot_len = res;
        pbuf_add_header(out, SPN_PDU_HDR_SIZE);

      } else if (hdr->service_id == DCP_SRV_ID_SET && hdr->service_type == DCP_SRV_TYPE_RES) {
        return -SPN_ENOSYS;
      } else if (hdr->service_id == DCP_SRV_ID_GET && hdr->service_type == DCP_SRV_TYPE_RES) {
        /* TODO: implement this */
        res = dcp_srv_set_cnf(ctx, &ctx->ucs_ctx, hdr, rtc_pdu->tot_len - 2);
        if (res != SPN_OK) {
          return res;
        }
        /* TODO: callback or notify set.req is done */
      } else {
        return -SPN_EINVAL;
      }

      /* If we need to reply */
      if (out) {
        struct db_interface* db_interface;
        struct db_port* db_port;
        struct db_object* obj;

        res = db_get_interface(ctx->db, ctx->interface_id, &db_interface);
        SPN_ASSERT("db_get_interface failed", res == SPN_OK);

        *PTR_OFFSET(out->payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);
        /* TODO: chose iface by LLDP if is unicast frame */
        if (1 /*is_multicast_addr(&mcr_ctx->src_addr) */) {
          for (idx = 0; idx < ARRAY_SIZE(db_interface->ports); idx++) {
            res = db_get_port(db_interface, idx, &db_port);
            if (res != SPN_OK) {
              continue;
            }

            res = db_get_object(&(db_port->objects), DB_ID_IFACE, &obj);
            SPN_ASSERT("db_get_object failed", res == SPN_OK);

            res = dcp_output(ctx, (struct spn_iface*)obj->data.ptr, src, out);
            SPN_ASSERT("dcp_output failed", res == SPN_OK);
          }
        }
        pbuf_free(out);
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
        sys_timeout(ctx->mcr_ctx[idx].response_delay, dcp_mcr_rsp_callback, &ctx->mcr_ctx[idx]);
      }
      break;
    default:
      return -SPN_EINVAL;
  }
  return SPN_OK;
}
