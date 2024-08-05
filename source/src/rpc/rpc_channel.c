#include <spn/errno.h>
#include <spn/rpc.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct rpc_channel* rpc_channel_alloc(struct rpc_ctx* ctx) {
  unsigned idx;

  for (idx = 0; idx < ARRAY_SIZE(ctx->channels); idx++) {
    if (ctx->channels[idx].state == RPC_CHANNEL_STATE_FREE) {
      ctx->channels[idx].state = RPC_CHANNEL_STATE_IDEL;
      return &ctx->channels[idx];
    }
  }
  return NULL;
}

void rpc_channel_free(struct rpc_channel* ch) {
  ch->state = RPC_CHANNEL_STATE_FREE;
}

struct rpc_channel* rpc_channel_find_by_uuid(struct rpc_ctx* ctx, rpc_uuid_t* uuid) {
  unsigned idx;

  for (idx = 0; idx < ARRAY_SIZE(ctx->channels); idx++) {
    if (ctx->channels[idx].state == RPC_CHANNEL_STATE_IDEL &&
        !memcmp(&ctx->channels[idx].act_uuid, uuid, sizeof(rpc_uuid_t))) {
      return &ctx->channels[idx];
    }
  }
  return NULL;
}

struct rpc_channel* rpc_channel_find_by_idx(struct rpc_ctx* ctx, int idx) {
  if (idx < 0 || (unsigned)idx >= ARRAY_SIZE(ctx->channels)) {
    return NULL;
  }
  return &ctx->channels[idx];
}

int rpc_get_client_channel(struct rpc_ctx* ctx, rpc_if_type_t remote_type, uint32_t remote_ip, uint16_t remote_port) {
  struct rpc_channel* ch;
  unsigned idx;

  for (idx = 0; idx < ARRAY_SIZE(ctx->channels); idx++) {
    ch = &ctx->channels[idx];
    if (ch->state == RPC_CHANNEL_STATE_FREE) {
      /* Basic setup for channel */
      ch->is_server = 0;
      ch->remote_ip = remote_ip;
      ch->remote_port = remote_port;
      memcpy(&ch->if_uuid, rpc_get_consistent_uuid(remote_type), sizeof(rpc_uuid_t));
      memcpy(&ch->act_uuid, ctx->act_uuid, sizeof(rpc_uuid_t));
      return idx;
    }
  }
  return -1;
}
