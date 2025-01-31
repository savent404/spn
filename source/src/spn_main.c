#include <netif/ethernet.h>
#include <spn/db_ll.h>
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
  struct db_object* obj;

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

      /**
       * DB_ID_NAME_OF_PORT
       * @note involve sprintf cost too much RO space
       * @todo only supported str[8]:port-xyz,
         need implement more like str[14]:port-xyz-rstuv */
      memcpy(&val.str, "port-xyz", 8);
      res = i * SPN_CONF_MAX_PORT_PER_INTERFACE + j + 1;
      val.str[5] = '0' + res / 100;
      val.str[6] = '0' + (res % 100) / 10;
      val.str[7] = '0' + res % 10;
      res = db_add_object(&ctx->db.interfaces[i].ports[j].objects, DB_ID_NAME_OF_PORT, 0, 1, 8, &val);
      SPN_ASSERT("Failed to add port name", res == SPN_OK);
    }

    if (!valid_port) {
      SPN_DEBUG_MSG(SPN_DEBUG, "No valid port found for interface %d\n", i);
      res = -SPN_ENOENT;
      goto err_ret;
    }

    /* Interface general attributes */
    /**
     * DB_ID_BLOCK_INFO
     * @note only support static ip now, shall be modified by DHCP service
     */
    val.u16 = IP_BLOCK_INFO_STATIC;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_BLOCK_INFO, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add ip block info", res == SPN_OK);

    /**
     * DB_ID_MAC_ADDR
     * @note interface is abstracted as a ethernet device, each port share the same attributes
     */
    memcpy(&val.str, ctx->ifaces[i][0].netif.hwaddr, 6);
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_MAC_ADDR, 0, 0, 6, &val);
    SPN_ASSERT("Failed to add mac addr", res == SPN_OK);

    /**
     * DB_ID_IP_ADDR
     * @note interface is abstracted as a ethernet device, each port share the same attributes
     */
    val.u32 = ctx->ifaces[i][0].netif.ip_addr.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_ADDR, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add ip addr", res == SPN_OK);

    /**
     * DB_ID_IP_MASK
     * @note interface is abstracted as a ethernet device, each port share the same attributes
     */
    val.u32 = ctx->ifaces[i][0].netif.netmask.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_MASK, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add netmask", res == SPN_OK);

    /**
     * DB_ID_IP_GATEWAY
     * @note interface is abstracted as a ethernet device, each port share the same attributes
     */
    val.u32 = ctx->ifaces[i][0].netif.gw.addr;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_IP_GATEWAY, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add gateway", res == SPN_OK);

    /**
     * DB_ID_NAME_OF_VENDOR
     * @note read only
     */
    SPN_ASSERT("Invalid vendor name", cfg->vendor_name);
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_VENDOR, 0, 0, 0, &val);
    SPN_ASSERT("Failed to add vendor name", res == SPN_OK);
    res = db_get_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_VENDOR, &obj);
    SPN_ASSERT("Failed to get vendor name", res == SPN_OK);
    res = db_dup_str2obj(obj, cfg->vendor_name, strlen(cfg->vendor_name));
    SPN_ASSERT("Failed to set vendor name", res == SPN_OK);

    /**
     * DB_ID_DEVICE_ID
     * @note read only
     */
    val.u16 = cfg->device_id;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_DEVICE_ID, 0, 1, sizeof(val), &val);
    SPN_ASSERT("Failed to add device id", res == SPN_OK);

    /**
     * DB_ID_VENDOR_ID
     * @note read only
     */
    val.u16 = cfg->vendor_id;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_VENDOR_ID, 0, 1, sizeof(val), &val);
    SPN_ASSERT("Failed to add vendor id", res == SPN_OK);

    /**
     * DB_ID_VENDOR_ID
     * @note read only
     */
    val.u8 = cfg->role;
    res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_DEVICE_ROLE, 0, 0, sizeof(val), &val);
    SPN_ASSERT("Failed to add role", res == SPN_OK);

    /**
     * DB_ID_NAME_OF_INTERFACE
     */
    if (cfg->station_name) {
      res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_INTERFACE, 0, 0, 0, &val);
      SPN_ASSERT("Failed to add station name", res == SPN_OK);
      res = db_get_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_INTERFACE, &obj);
      SPN_ASSERT("Failed to get station name", res == SPN_OK);
      res = db_dup_str2obj(obj, cfg->station_name, strlen(cfg->station_name));
      SPN_ASSERT("Failed to set station name", res == SPN_OK);
    } else {
      val.str[0] = '\0';
      res = db_add_object(&ctx->db.interfaces[i].objects, DB_ID_NAME_OF_INTERFACE, 0, 1, 0, &val);
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
  if (res == SPN_OK) {
    pbuf_free(p);
  }
  return res;
}

int _spn_input_hook(struct spn_ctx* ctx, struct pbuf* p, struct spn_iface* iface) {
  struct eth_addr src, dst;
  uint16_t eth_type, frame_id;
  int res = -1;

  /* FIXME: This is not compatable for VLAN */
  memcpy(&dst, ((const char*)p->payload), sizeof(dst));
  memcpy(&src, ((const char*)p->payload) + 6, sizeof(src));
  eth_type = (*PTR_OFFSET(p->payload, 12, uint16_t));

  if (eth_type == SPN_HTONS(ETHTYPE_PROFINET)) {
    pbuf_remove_header(p, SIZEOF_ETH_HDR);
    frame_id = SPN_NTOHS(*PTR_OFFSET(p->payload, 0, uint16_t));
    switch (frame_id) {
      case FRAME_ID_DCP_IDENT_REQ:
      case FRAME_ID_DCP_IDENT_RES:
      case FRAME_ID_DCP_GET_SET:
      case FRAME_ID_DCP_HELLO_REQ:
        res = dcp_input(&ctx->dcp, iface, &dst, &src, p->payload, p->tot_len);
        break;
      default:
        res = 0;
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
  return -SPN_ENOENT;
}

__attribute__((weak)) void _spn_input_indication(int result) {
  SPN_UNUSED_ARG(result);
}
