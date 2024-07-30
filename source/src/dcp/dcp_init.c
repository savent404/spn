#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <string.h>

void dcp_init(struct dcp_ctx* ctx, struct db_ctx* db) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->db = db;

  ctx->mcs_ctx.xid = 0x88000000;
  ctx->mcs_ctx.response_delay_factory = 1;
  ctx->mcs_ctx.external_interface_id = SPN_EXTERNAL_INTERFACE_BASE;
  ctx->ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
}
