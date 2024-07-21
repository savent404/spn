#include <spn/spn.h>
#include <spn/sys.h>

int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg) {
  int res, i, j;
  db_value_t val;

  SPN_UNUSED_ARG(cfg);

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

err_ret:
  db_deinit(&ctx->db);
  return res;
}

void spn_deinit(struct spn_ctx* ctx) {
  dcp_deinit(&ctx->dcp);
  db_deinit(&ctx->db);
}

int spn_input_hook(void* frame, void* iface) {
  SPN_UNUSED_ARG(frame);
  SPN_UNUSED_ARG(iface);
  _spn_input_indication(0);
  return 0;
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
