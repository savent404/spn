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

// clang-format off
const rpc_uuid_t device_interface = {
    .form = {
        .data1 = 0xDEA00000,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t controller_interafce = {
    .form = {
        .data1 = 0xDEA00001,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t supervisor_interface = {
    .form = {
        .data1 = 0xDEA00002,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t parameter_server_interface = {
    .form = {
        .data1 = 0xDEA00003,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t cim_interface = {
    .form = {
        .data1 = 0xDEA00004,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t epmap_interface = {
    .form = {
        .data1 = 0x8A885D04,
        .data2 = 0x1CEB,
        .data3 = 0x11C9,
        .data4 = {0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60}
    }
};

const rpc_uuid_t epmap_object = {
    .form = {
        .data1 = 0x8A885D04,
        .data2 = 0x1CEB,
        .data3 = 0x11C9,
        .data4 = {0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60}
    }
};
// clang-format on

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
    memcpy(&ch->act_uuid, &hdr->object_uuid, sizeof(rpc_uuid_t));
    ch->remote_ip = remote_ip;
    ch->remote_port = remote_port;
    ch->host_port = host_port;
    ch->is_server = 1;
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
