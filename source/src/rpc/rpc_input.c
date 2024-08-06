#include <spn/config.h>
#include <spn/errno.h>
#include <spn/rpc.h>
#include <spn/sys.h>
#include <string.h>

#if 0
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#error "big endian is not supported"
#endif
#endif

int rpc_input(struct rpc_ctx* ctx,
              void* payload,
              int length,
              uint32_t remote_ip,
              uint16_t remote_port,
              uint16_t host_port) {
  struct rpc_hdr* hdr;
  struct rpc_channel* ch;
  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  SPN_ASSERT("invalid ctx", ctx);
  SPN_ASSERT("invalid payload", payload && length);

  hdr = (struct rpc_hdr*)payload;

  rpc_hdr_hton(hdr);

  /* TODO: general check for rpc header */

  ch = rpc_channel_find_by_uuid(ctx, &hdr->object_uuid);
  if (!ch) {
    ch = rpc_channel_alloc(ctx);
    if (!ch) {
      SPN_DEBUG_MSG(SPN_RPC_DEBUG, "No free channel\n");
      return -SPN_ENOMEM;
    }
    memcpy(&ch->act_uuid, &hdr->activity_uuid, sizeof(rpc_uuid_t));
    memcpy(&ch->if_uuid, &hdr->interface_uuid, sizeof(rpc_uuid_t));
    ch->remote_ip = remote_ip;
    ch->remote_port = remote_port;
    ch->host_port = host_port;
    ch->is_server = 1;
    ch->is_le = !rpc_is_big_endian(hdr);
    memcpy(&ch->input_buf, payload, length);
    ch->input_len = length;
    ch->output_len = 0;
    /* TODO: more attributes need to be inited */
  }

  SPN_ASSERT("No frag support", hdr->frag_numb == 0);

  switch (hdr->packet_type) {
    case RPC_PKT_TYPE_REQ: {
      SPN_DEBUG_MSG(SPN_RPC_DEBUG, "rpc req\n");
      struct rpc_ndr_data_req* req_hdr = (struct rpc_ndr_data_req*)hdr->ndr_data;
      if (rpc_is_big_endian(hdr)) {
        rpc_ndr_hton(req_hdr, RPC_PKT_TYPE_REQ);
      }
      SPN_ASSERT("Stupid RPC, use 2 attrs to present the same value", req_hdr->maxium_count == req_hdr->args_maxium);
      SPN_ASSERT("Stupid RPC, offset always be zero", req_hdr->offset == 0);
      ctx->fn_rpc_req(ch, hdr->ndr_data, req_hdr->args_length);
      break;
    }
    case RPC_PKT_TYPE_RESP: {
      SPN_DEBUG_MSG(SPN_RPC_DEBUG, "rpc rsp\n");
      ctx->fn_rpc_rsp(ch, NULL, 0);
      break;
    }
    case RPC_PKT_TYPE_PING: {
      SPN_DEBUG_MSG(SPN_RPC_DEBUG, "rpc ping\n");
      ctx->fn_rpc_ping(ch, NULL, 0);
      break;
    }
    default:
      SPN_DEBUG_MSG(SPN_RPC_DEBUG, "Unknown rpc packet type: %02X\n", hdr->packet_type);
      break;
  }

  return 0;
}
