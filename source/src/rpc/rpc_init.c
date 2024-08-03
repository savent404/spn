#include <spn/rpc.h>
#include <string.h>
#include <spn/errno.h>

int rpc_init(struct rpc_ctx *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    return SPN_OK;
}
