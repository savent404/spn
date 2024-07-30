#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/iface.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <spn/timeout.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))
#define BLOCK_TYPE(h, l) ((h << 8) | l)

static void dcp_mcr_rsp_callback(void* arg) {
  struct dcp_mcr_ctx* mcr_ctx = (struct dcp_mcr_ctx*)arg;
  struct dcp_ctx* ctx = mcr_ctx->dcp_ctx;
  struct db_interface* db_interface;
  struct db_port* db_port;
  struct db_object* obj;
  spn_frame_t out;
  int res;
  uint16_t length;
  unsigned idx;

  res = db_get_interface(ctx->db, ctx->interface_id, &db_interface);
  SPN_ASSERT("db_get_interface failed", res == SPN_OK);

  out = spn_alloc_frame(FRAME_TYPE_DCP);
  SPN_ASSERT("spn_alloc_frame failed", out);

  res = dcp_srv_ident_rsp(ctx, mcr_ctx, spn_frame_data(out), &length);
  SPN_ASSERT("dcp_srv_ident_rsp failed", res == SPN_OK);
  spn_frame_set_size(out, length);

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

  spn_free_frame(out);
}

int dcp_input(struct dcp_ctx* ctx,
              struct spn_iface* iface,
              const struct eth_addr* dst,
              const struct eth_addr* src,
              void* pdu,
              uint16_t length) {
  uint16_t frame_id;
  struct dcp_header* dcp_hdr;
  struct pbuf* out = NULL;
  uint16_t ex_iface;
  struct db_interface* db_iface;
  struct db_object* db_obj;
  int res;
  unsigned idx, dcp_length;

  SPN_UNUSED_ARG(iface);
  SPN_UNUSED_ARG(dst);

  frame_id = SPN_NTOHS(*PTR_OFFSET(pdu, 0, uint16_t));
  dcp_hdr = PTR_OFFSET(pdu, 2, struct dcp_header);
  dcp_length = length - SPN_PDU_HDR_SIZE;

  switch (frame_id) {
    case FRAME_ID_DCP_HELLO_REQ:
    case FRAME_ID_DCP_IDENT_RES:
      if (dcp_hdr->service_id != DCP_SRV_ID_IDENT && dcp_hdr->service_type != DCP_SRV_TYPE_RES) {
        return -SPN_EINVAL;
      }
      /**
       * @brief call ident.cnf and fill the mac address info if device did not response it
       */
      res = dcp_srv_ident_cnf(ctx, &ctx->mcs_ctx, dcp_hdr, dcp_length, &ex_iface);
      SPN_ASSERT("dcp_srv_ident_cnf failed", res == SPN_OK);

      if (db_get_interface_object(ctx->db, ex_iface, DB_ID_IP_MAC_ADDR, &db_obj) == -SPN_ENOENT) {
        db_value_t val;
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "device didn't response mac address, fill it based on ethframe\n");
        memcpy(val.mac, src->addr, sizeof(val.mac));

        res = db_get_interface(ctx->db, ex_iface, &db_iface);
        SPN_ASSERT("db_get_interface failed", res == SPN_OK);
        res = db_add_object(&db_iface->objects, DB_ID_IP_MAC_ADDR, 0, 0, sizeof(val.mac), &val);
        SPN_ASSERT("db_add_object failed", res == SPN_OK);
      }
      break;
    case FRAME_ID_DCP_GET_SET:
      /* TODO: if in operating mode, should ignore this request */
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: dcp_input: get/set srv: %02d:%02d\n", dcp_hdr->service_id,
                    dcp_hdr->service_type);
      if (dcp_hdr->service_id == DCP_SRV_ID_GET && dcp_hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;
        res = dcp_srv_get_ind(ctx, &ucr, dcp_hdr, dcp_length);
        SPN_ASSERT("dcp_srv_get_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_LINK, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        res = dcp_srv_get_rsp(ctx, &ucr, out->payload, &out->tot_len);
        SPN_ASSERT("dcp_srv_set_rsp failed", res == SPN_OK);
        pbuf_add_header(out, -SPN_PDU_HDR_SIZE);

      } else if (dcp_hdr->service_id == DCP_SRV_ID_SET && dcp_hdr->service_type == DCP_SRV_TYPE_REQ) {
        struct dcp_ucr_ctx ucr;

        res = dcp_srv_set_ind(ctx, &ucr, dcp_hdr, dcp_length);
        SPN_ASSERT("dcp_srv_set_ind failed", res == SPN_OK);

        out = pbuf_alloc(PBUF_LINK, SPN_DCP_MAX_SIZE + SPN_PDU_HDR_SIZE, PBUF_RAM);
        SPN_ASSERT("pbuf_alloc failed", out != NULL);

        res = dcp_srv_set_rsp(ctx, &ucr, out->payload, &out->tot_len);
        SPN_ASSERT("dcp_srv_set_rsp failed", res == SPN_OK);

      } else if (dcp_hdr->service_id == DCP_SRV_ID_SET && dcp_hdr->service_type == DCP_SRV_TYPE_RES) {
        res = dcp_srv_set_cnf(ctx, &ctx->ucs_ctx, dcp_hdr, dcp_length);
        if (res != SPN_OK) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: dcp_input: set.cnf failed: %d\n", res);
          return res;
        }
        /* TODO: callback or notify set.req is done */
      } else if (dcp_hdr->service_id == DCP_SRV_ID_GET && dcp_hdr->service_type == DCP_SRV_TYPE_RES) {
        res = dcp_srv_get_cnf(ctx, &ctx->ucs_ctx, dcp_hdr, dcp_length);
        if (res != SPN_OK) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: dcp_input: get.cnf failed: %d\n", res);
          return res;
        }
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
          res = dcp_srv_ident_ind(ctx, &ctx->mcr_ctx[idx], dcp_hdr, dcp_length);
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
        SPN_TIMEOUT(ctx->mcr_ctx[idx].response_delay, dcp_mcr_rsp_callback, &ctx->mcr_ctx[idx]);
      }
      break;
    default:
      return -SPN_EINVAL;
  }
  return SPN_OK;
}
