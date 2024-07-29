#include "app.h"
#include <assert.h>
#include <lwip/debug.h>
#include <lwip/ip.h>
#include <lwip/ip_addr.h>
#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/spn.h>
#include <stdio.h>
#include <string.h>

enum app_state {
  APP_STATE_INIT,
  APP_STATE_DCP_IDENT,
  APP_STATE_DCP_SET,
  APP_STATE_DCP_CONFIRM,
  APP_STATE_PRM,
  APP_STATE_RUN,
};

struct app_instance {
  struct spn_ctx* ctx;
  const struct spn_cfg* cfg;
  enum app_state state;
};

static void app_rta_timer_handler(void* arg) {
  struct app_instance* inst = (struct app_instance*)arg;
  int res;
  unsigned next_time = 1000;
  printf("rta timer handler\n");

  switch (inst->state) {
    case APP_STATE_INIT:
      printf(">>> dcp init");
      /** Discovery DCP devices, filted by station name */
      // TODO: topology should be well defined
      inst->ctx->dcp.mcs_ctx.station_name = "et200ecopn.dev5";
      inst->ctx->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
      inst->ctx->dcp.mcs_ctx.response_delay_factory = 1;
      inst->ctx->dcp.mcs_ctx.xid = 0x88000000;
      {
        struct pbuf* p = pbuf_alloc(PBUF_LINK, 1500, PBUF_RAM);
        struct eth_addr dst_addr = {.addr = {0x01, 0x0e, 0xcf, 00, 00, 00}};
        unsigned i;
        uint16_t length;

        dcp_srv_ident_req(&inst->ctx->dcp, &inst->ctx->dcp.mcs_ctx, p->payload, &length);
        p->tot_len = length;

        for (i = 0; i < ARRAY_SIZE(inst->ctx->ifaces[0]); i++) {
          spn_iface_t* iface = &inst->ctx->ifaces[0][i];
          if (!iface->netif.output) {
            continue;
          }
          res = dcp_output(&inst->ctx->dcp, iface, &dst_addr, p);
          assert(res == SPN_OK);
        }
        pbuf_free(p);
      }
      inst->state = APP_STATE_DCP_IDENT;
      next_time = inst->ctx->dcp.mcs_ctx.response_delay + 100;  // we need some mercy time
      break;
    case APP_STATE_DCP_IDENT:
      printf(">>> dcp state\n");
      // make sure that ident.req context is cleared, and device should be located in the db
      assert(inst->ctx->dcp.mcs_ctx.req_options_bitmap == 0);
      assert(inst->ctx->dcp.mcs_ctx.state == DCP_STATE_IDLE);
      assert(inst->ctx->dcp.mcs_ctx.xid == 0x88000001);
      assert(inst->ctx->dcp.mcs_ctx.external_interface_id == SPN_EXTERNAL_INTERFACE_BASE + 1);
      {
        struct db_interface* iface;
        struct db_object* obj;
        char* station_name;

        res = db_get_interface(&inst->ctx->db, SPN_EXTERNAL_INTERFACE_BASE, &iface);
        assert(res == SPN_OK);

        res = db_get_object(&iface->objects, DB_ID_NAME_OF_INTERFACE, &obj);
        assert(res == SPN_OK);

        station_name = (char*)malloc(obj->attr.len + 1);
        if (obj->attr.is_dyn) {
          memcpy(station_name, obj->data.ptr, obj->attr.len);
          station_name[obj->attr.len] = '\0';
        } else {
          memcpy(station_name, obj->data.str, obj->attr.len);
          station_name[obj->attr.len] = '\0';
        }
        printf("station name: %s\n", station_name);
        free(station_name);

        res = db_get_object(&iface->objects, DB_ID_VENDOR_ID, &obj);
        if (res != SPN_OK) {
          printf("vendor id not found\n");
        } else {
          printf("vendor id: %04x\n", obj->data.u16);
        }

        res = db_get_object(&iface->objects, DB_ID_DEVICE_ID, &obj);
        if (res != SPN_OK) {
          printf("device id not found\n");
        } else {
          printf("device id: %04x\n", obj->data.u16);
        }

        res = db_get_object(&iface->objects, DB_ID_DEVICE_ROLE, &obj);
        if (res != SPN_OK) {
          printf("device role not found\n");
        } else {
          printf("device role: %02x\n", obj->data.u8);
        }

        res = db_get_object(&iface->objects, DB_ID_IP_ADDR, &obj);
        if (res != SPN_OK) {
          printf("ip address not found\n");
        } else {
          printf("device ip: %s\n", ip4addr_ntoa((ip4_addr_t*)&obj->data.u32));
          inst->ctx->dcp.ucs_ctx.ip_addr = obj->data.u32;
        }

        res = db_get_object(&iface->objects, DB_ID_IP_MASK, &obj);
        if (res != SPN_OK) {
          printf("subnet mask not found\n");
        } else {
          printf("subnet mask: %s\n", ip4addr_ntoa((ip4_addr_t*)&obj->data.u32));
          inst->ctx->dcp.ucs_ctx.ip_mask = obj->data.u32;
        }

        res = db_get_object(&iface->objects, DB_ID_IP_GATEWAY, &obj);
        if (res != SPN_OK) {
          printf("gateway not found\n");
        } else {
          printf("gateway: %s\n", ip4addr_ntoa((ip4_addr_t*)&obj->data.u32));
          inst->ctx->dcp.ucs_ctx.ip_gw = obj->data.u32;
        }
      }

      inst->state = APP_STATE_DCP_SET;
      break;
    case APP_STATE_DCP_SET:
      printf(">>> dcp set state\n");

      inst->ctx->dcp.ucs_ctx.xid = 0xFFAA;
      inst->ctx->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_IP_PARAMETER;
      {
        uint8_t i = inst->ctx->dcp.ucs_ctx.ip_addr >> 24;
        i = (i - 20 + 1) % 20 + 20;  // range: [20,40), incremental
        inst->ctx->dcp.ucs_ctx.ip_addr &= 0x00FFFFFF;
        inst->ctx->dcp.ucs_ctx.ip_addr |= i << 24;
      }
      {
        struct pbuf* p = pbuf_alloc(PBUF_LINK, 1500, PBUF_RAM);
        struct db_object* obj;
        struct spn_iface* iface;
        struct eth_addr addr;
        res = dcp_srv_set_req(&inst->ctx->dcp, &inst->ctx->dcp.ucs_ctx, p->payload, &p->tot_len);
        assert(res == SPN_OK);

        /* TODO: find the right port to send package */
        res = db_get_port_object(&inst->ctx->db, 0, 0, DB_ID_IFACE, &obj);
        assert(res == SPN_OK);
        iface = obj->data.ptr;

        res = db_get_interface_object(&inst->ctx->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MAC_ADDR, &obj);
        assert(res == SPN_OK);
        memcpy(addr.addr, obj->data.mac, sizeof(addr.addr));

        res = dcp_output(&inst->ctx->dcp, iface, &addr, p);
        assert(res == SPN_OK);

        pbuf_free(p);
      }

      next_time = SPN_DCP_UC_TIMEOUT;
      inst->state = APP_STATE_DCP_CONFIRM;
      break;
    case APP_STATE_DCP_CONFIRM:
      printf(">>> dcp confirm state\n");
      inst->ctx->dcp.ucs_ctx.req_options_bitmap = (1 << DCP_BIT_IDX_IP_MAC_ADDRESS) | (1 << DCP_BIT_IDX_IP_PARAMETER) |
                                                  (1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION);
      inst->ctx->dcp.ucs_ctx.xid = 0xFFAA;

      {
        struct pbuf* p = pbuf_alloc(PBUF_LINK, 1500, PBUF_RAM);
        struct db_object* obj;
        struct spn_iface* iface;
        struct eth_addr addr;

        res = dcp_srv_get_req(&inst->ctx->dcp, &inst->ctx->dcp.ucs_ctx, p->payload, &p->tot_len);
        assert(res == SPN_OK);

        /* TODO: find the right port to send package */
        res = db_get_port_object(&inst->ctx->db, 0, 0, DB_ID_IFACE, &obj);
        assert(res == SPN_OK);
        iface = obj->data.ptr;

        res = db_get_interface_object(&inst->ctx->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MAC_ADDR, &obj);
        assert(res == SPN_OK);
        memcpy(addr.addr, obj->data.mac, sizeof(addr.addr));

        res = dcp_output(&inst->ctx->dcp, iface, &addr, p);
        assert(res == SPN_OK);

        pbuf_free(p);
      }
      next_time = SPN_DCP_UC_TIMEOUT;
      inst->state = APP_STATE_PRM;
      break;
    case APP_STATE_PRM:
      printf(">>> prm state\n");
      inst->state = APP_STATE_RUN;
      break;
    case APP_STATE_RUN:
      break;
    default:
      break;
  }

  sys_timeout(next_time, app_rta_timer_handler, inst);
}

int app_init(struct spn_ctx* ctx, const struct spn_cfg* cfg) {
  static struct app_instance app_inst = {0};
  printf("every thing goes well, try to register devices...\n");

  app_inst.ctx = ctx;
  app_inst.cfg = cfg;
  app_inst.state = APP_STATE_INIT;

  // go to timer context
  LOCK_TCPIP_CORE();
  sys_timeout(1000, app_rta_timer_handler, &app_inst);
  UNLOCK_TCPIP_CORE();
  return 0;
}
