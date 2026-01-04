#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <string.h>

/*@
  @ requires \valid(ctx);
  @ requires \valid(db);
  @ requires \separated(ctx, db);
  @ assigns *ctx;
  @ ensures ctx->db == db;
  @ ensures ctx->mcs_ctx.xid == 0x88000000;
  @ ensures ctx->mcs_ctx.response_delay_factory == 1;
  @ ensures ctx->mcs_ctx.external_interface_id == SPN_EXTERNAL_INTERFACE_BASE;
  @ ensures ctx->ucs_ctx.ex_ifr == SPN_EXTERNAL_INTERFACE_BASE;
  @*/
void dcp_init(struct dcp_ctx* ctx, struct db_ctx* db) {
#if 0
  // Can't prove memset sets all fields to zero
  memset(ctx, 0, sizeof(*ctx));
#else
  ctx->db = NULL;
  ctx->port_num = 0;
  ctx->port_num = 0;
  ctx->ucs_ctx = (struct dcp_ucs_ctx){0};
  ctx->mcs_ctx = (struct dcp_mcs_ctx){0};
  ctx->mcr_ctx[0] = (struct dcp_mcr_ctx){0};
  // @assert(ARRAY_SIZE(ctx->mcr_ctx) == 1);
#endif
  ctx->db = db;

  ctx->mcs_ctx.xid = 0x88000000;
  ctx->mcs_ctx.response_delay_factory = 1;
  ctx->mcs_ctx.external_interface_id = SPN_EXTERNAL_INTERFACE_BASE;
  ctx->ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
}
