#include <lwip/pbuf.h>
#include <lwip/sockets.h>
#include <spn/rpc.h>
#include <spn/sys.h>

static inline int get_buf_offset(void* p, void* buf, int buf_len) {
  uintptr_t p_addr = (uintptr_t)p;
  uintptr_t buf_addr = (uintptr_t)buf;

  if (p_addr < buf_addr || p_addr >= buf_addr + buf_len) {
    return -1;
  }
  return p_addr - buf_addr;
}

int rpc_output(struct rpc_channel* ch, void* _p, int _l) {
  struct rpc_hdr* out_hdr;
  struct rpc_hdr* input_hdr = (struct rpc_hdr*)ch->input_buf;
  int offset, length;

  /* Detect upper is using inner or outside buffer */
  offset = get_buf_offset(_p, ch->output_buf, ch->output_len);
  if (offset < 0) {
    memcpy(ch->output_buf + sizeof(*out_hdr), _p, _l);
    out_hdr = (struct rpc_hdr*)ch->output_buf;
  } else {
    out_hdr = (struct rpc_hdr*)((char*)_p - sizeof(*out_hdr));
  }
  length = _l + sizeof(*out_hdr);
  SPN_ASSERT("Invalid usage of output_buf\n", offset < (int)sizeof(*out_hdr));

  /* Fill all the rpc hdr, many attributes are the same as the input hdr. dont wast time on it */
  if (ch->is_server) {
    memcpy(out_hdr, input_hdr, sizeof(struct rpc_hdr));
  } else {
    out_hdr->version = 4;
    out_hdr->drep1 = ch->is_le ? 0x10 : 0x00;
    out_hdr->drep2 = 0x00;
    out_hdr->drep3 = 0x00;
    out_hdr->serial_high = 0;
    memcpy(&out_hdr->object_uuid, rpc_get_object_uuid(), sizeof(rpc_uuid_t));
    memcpy(&out_hdr->interface_uuid, &ch->if_uuid, sizeof(rpc_uuid_t));
    memcpy(&out_hdr->activity_uuid, &ch->act_uuid, sizeof(rpc_uuid_t));
    out_hdr->interface_version_major = 0;
    out_hdr->interface_version_minor = 1;
    out_hdr->seq_numb = 0; /* FIXME: support frag */
    out_hdr->operation_numb = ch->req_op;
    out_hdr->interface_hint = 0xFFFF;
    out_hdr->activity_hint = 0xFFFF;
    out_hdr->auth_protocol = 0;
    out_hdr->serial_low = 0;
  }

  out_hdr->packet_type = ch->rsp_pkt_type;

  /** FIXME: Shold support frag freature */
  SPN_ASSERT("Not support frag yet\n", ch->output_len < 1500);
  if (ch->is_server) {
    out_hdr->flag1 = RPC_FLAG1_NO_FACK | RPC_FLAG1_LAST_FRAG;
  } else {
    out_hdr->flag1 = RPC_FLAG1_IDEMPOTENT;
  }
  out_hdr->flag2 = 0x00;
  /* TODO: support boot time */
  out_hdr->boot_time = 0;

  /* TODO: support frag */
  out_hdr->frag_numb = 0;

  out_hdr->length_of_body = length - sizeof(*out_hdr);

  rpc_hdr_ntoh(out_hdr);

  SPN_ASSERT("Invalid context", ch->ctx);
  SPN_ASSERT("Invalid rpc output fn", ch->ctx->fn_udp_out);
  return ch->ctx->fn_udp_out(out_hdr, length, ch->remote_ip, ch->remote_port, ch->host_port);
}
