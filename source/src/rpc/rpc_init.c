#include <spn/errno.h>
#include <spn/rpc.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int rpc_init(struct rpc_ctx* ctx) {
  unsigned i;
  memset(ctx, 0, sizeof(*ctx));

  for (i = 0; i < ARRAY_SIZE(ctx->channels); i++) {
    ctx->channels[i].ctx = ctx;
  }
  return SPN_OK;
}
