#include <netif/ethernet.h>
#include <spn/pdu.h>
#include <spn/spn.h>
#include <spn/sys.h>
#include <stdlib.h>
#include <string.h>

#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

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
    int valid_port = 0;
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
      valid_port++;

      res = db_add_port(&ctx->db.interfaces[i], j);
      SPN_ASSERT("Failed to add port", res == SPN_OK);

      /* All things done, register iface into db */
      val.ptr = &ctx->ifaces[i][j];
      res = db_add_object(&ctx->db.interfaces[i].ports[j].objects, DB_ID_IFACE, 0, 0, sizeof(val), &val);
      SPN_ASSERT("Failed to add iface to db", res == SPN_OK);
    }
    if (!valid_port) {
      SPN_DEBUG_MSG(SPN_DEBUG, "No valid port found for interface %d\n", i);
      res = -SPN_ENOENT;
      goto err_ret;
    }

    val.u16 = 1;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_BLOCK_INFO, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add ip block info", res == SPN_OK);

    val.u32 = ctx->ifaces[i][0].netif.ip_addr.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_ADDR, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add ip addr", res == SPN_OK);

    val.u32 = ctx->ifaces[i][0].netif.netmask.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_MASK, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add netmask", res == SPN_OK);

    val.u32 = ctx->ifaces[i][0].netif.gw.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_GATEWAY, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add gateway", res == SPN_OK);

    SPN_ASSERT("Invalid vendor name", cfg->vendor_name);
    res = strlen(cfg->vendor_name);
    if (res < sizeof(val.str)) {
      strncpy(val.str, cfg->vendor_name, res);
      res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_VENDOR, 0, 1, res, &val);
    } else {
      val.ptr = strdup(cfg->vendor_name);
      res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_VENDOR, 0, 1, res, &val);
    }
    SPN_ASSERT("Failed to add vendor name", res == SPN_OK);

    val.u16 = cfg->device_id;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_DEVICE_ID, 0, 1, sizeof(val), &val);
    SPN_ASSERT("Failed to add device id", res == SPN_OK);

    val.u16 = cfg->vendor_id;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_VENDOR_ID, 0, 1, sizeof(val), &val);
    SPN_ASSERT("Failed to add vendor id", res == SPN_OK);

    if (cfg->station_name) {
      res = strlen(cfg->station_name);
      if (res < sizeof(val.str)) {
        strncpy(val.str, cfg->station_name, res);
        res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_STATION, 0, 1, res, &val);
      } else {
        val.ptr = strdup(cfg->station_name);
        res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_STATION, 0, 1, res, &val);
      }
    } else {
      val.str[0] = '\0';
      res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_STATION, 0, 1, 0, &val);
    }
    SPN_ASSERT("Failed to add station name", res == SPN_OK);
  }

  /* Now do the general things now */
  dcp_init(&ctx->dcp, &ctx->db);

  _holly_protected_ctx = ctx;

  return res;

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

int _spn_input_hook(struct spn_ctx* ctx, struct pbuf* p, struct spn_iface* iface) {
  struct eth_addr src;
  uint16_t eth_type, frame_id;
  int res = -1;

  pbuf_add_header(p, SIZEOF_ETH_HDR);
  memcpy(&src, p->payload, sizeof(src));
  eth_type = SPN_NTOHS(*PTR_OFFSET(p->payload, 12, uint16_t));
  pbuf_remove_header(p, SIZEOF_ETH_HDR);

  if (eth_type == ETHTYPE_PROFINET) {
    frame_id = SPN_NTOHS(*PTR_OFFSET(p->payload, 0, uint16_t));
    switch (frame_id) {
      case FRAME_ID_DCP_IDENT_REQ:
      case FRAME_ID_DCP_IDENT_RES:
      case FRAME_ID_DCP_GET_SET:
      case FRAME_ID_DCP_HELLO_REQ:
        res = dcp_input(&ctx->dcp, iface, &src, p);
        break;
      default:
        res = -1;
        SPN_DEBUG_MSG(SPN_DEBUG, "Unknown frame id %d\n", frame_id);
        break;
    }
  }
  return res;
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
