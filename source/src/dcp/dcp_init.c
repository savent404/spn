#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <string.h>

void dcp_init(struct dcp_ctx* ctx, struct db_ctx* db)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->db = db;

    ctx->cnf_interface_id = 100;
    ctx->cnf_delay_factory = 1;
    ctx->cnf_xid = 0x12345678;
}
