#include <spn/spn.h>
#include <spn/sys.h>

static struct spn_ctx* _holly_protected_ctx = NULL;

int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg) {
  int res, i, j;
  db_value_t val;

  SPN_UNUSED_ARG(cfg);

  if (_holly_protected_ctx) {
    SPN_DEBUG_MSG(SPN_DEBUG, "SPN already initialized\n");
    return -SPN_EBUSY;
  }

  db_init(&ctx->db);

  /* NOTE: Try to init iface things firstly, in case of the following error */
  for (i = 0; i < SPN_CONF_MAX_INTERFACE; i++) {
    res = db_add_interface(&ctx->db, i);
    SPN_ASSERT("Failed to add interface", res == SPN_OK);

    /* Add port */
    for (j = 0; j < SPN_CONF_MAX_PORT_PER_INTERFACE; j++) {
      res = spn_port_init(ctx, &ctx->ifaces[i][j], i, j);
      if (res == -SPN_ENOENT) {
        SPN_DEBUG_MSG(SPN_DEBUG, "Port %d:%d is not found, skiping...\n", i, j);
        continue;
      } else if (res != SPN_OK) {
        SPN_DEBUG_MSG(SPN_DEBUG, "Failed to add port %d:%d, res=%d\n", i, j, res);
        goto err_ret;
      }

      res = db_add_port(&ctx->db.interfaces[i], j);
      SPN_ASSERT("Failed to add port", res == SPN_OK);

      /* All things done, register iface into db */
      val.ptr = &ctx->ifaces[i][j];
      res = db_add_object(&ctx->db.interfaces[i].ports[j].objects, DB_ID_NAME_OF_PORT, 0, 0, sizeof(val), &val);
      SPN_ASSERT("Failed to add iface to db", res == SPN_OK);
    }
  }

  /* Now do the general things now */
  dcp_init(&ctx->dcp, &ctx->db);

  _holly_protected_ctx = ctx;

err_ret:
  db_deinit(&ctx->db);
  return res;
}

void spn_deinit(struct spn_ctx* ctx) {
  SPN_ASSERT("SPN not initialized", _holly_protected_ctx == ctx);
  dcp_deinit(&ctx->dcp);
  db_deinit(&ctx->db);
  _holly_protected_ctx = NULL;
}

int spn_input_hook(void* frame, void* iface) {
  struct pbuf* p = (struct pbuf*)frame;
  struct netif* netif = (struct netif*)iface;
  struct spn_ctx* ctx = _holly_protected_ctx;
  int res = -1;

  SPN_ASSERT("SPN not initialized", ctx);
  SPN_ASSERT("Invalid netif pointer",
             netif >= (struct netif*)ctx->ifaces && netif < (struct netif*)ctx->ifaces + sizeof(ctx->ifaces));

  res = _spn_input_hook(ctx, p, (struct spn_iface*)netif);
  _spn_input_indication(res);
  return res;
}

int _spn_input_hook(struct spn_ctx* ctx, struct pbuf* p, struct spn_iface* iface)
{
  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(p);
  SPN_UNUSED_ARG(iface);
  return -1;
}

__attribute((weak)) int spn_port_init(struct spn_ctx* ctx, struct spn_iface* iface, uint16_t interface, uint16_t port) {
  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(iface);
  SPN_UNUSED_ARG(interface);
  SPN_UNUSED_ARG(port);
  return SPN_OK;
}

__attribute__((weak)) void _spn_input_indication(int result) {
  SPN_UNUSED_ARG(result);
}
