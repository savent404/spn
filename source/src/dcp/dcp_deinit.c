#include <spn/dcp.h>

void dcp_deinit(struct dcp_ctx* ctx) {
  if (ctx) {
    ctx->db = NULL;
  }
}
