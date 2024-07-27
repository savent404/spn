#include <assert.h>
#include <lwip/debug.h>
#include <lwip/ip.h>
#include <lwip/ip_addr.h>
#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/spn.h>
#include "app.h"
#include <stdio.h>
#include <string.h>

enum app_state {
  APP_STATE_INIT,
  APP_STATE_DCP,
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
  unsigned next_time = 1000;
  printf("rta timer handler\n");

  switch (inst->state) {
    case APP_STATE_INIT:
      /** Discovery DCP devices, filted by station name */
      // TODO: topology should be well defined
      inst->ctx->dcp.mcs_ctx.station_name = "et200ecopn.dev3";
      inst->ctx->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION;
      {
        struct pbuf* p = pbuf_alloc(PBUF_LINK, 1500, PBUF_RAM);
        dcp_srv_ident_req(&inst->ctx->dcp, &inst->ctx->dcp.mcs_ctx, p);
        pbuf_free(p);
      }
      next_time = 500;  // device should response in 400ms, but we need some mercy time
      break;
    case APP_STATE_DCP:
      printf("dcp state\n");
      // make sure that ident.req context is cleared, and device should be located in the db
      assert(inst->ctx->dcp.mcs_ctx.req_options_bitmap == 0);
      assert(inst->ctx->dcp.mcs_ctx.state == DCP_STATE_IDLE);
      assert(inst->ctx->dcp.mcs_ctx.xid == 0x88000001);
      assert(inst->ctx->dcp.mcs_ctx.response_interface_id == SPN_EXTERNAL_INTERFACE_BASE + 1);
      {
        struct db_interface* iface;
        struct db_object* obj;
        char* station_name;
        int res;

        res = db_get_interface(&inst->ctx->db, SPN_EXTERNAL_INTERFACE_BASE + 1, &iface);
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
        }

        res = db_get_object(&iface->objects, DB_ID_IP_MASK, &obj);
        if (res != SPN_OK) {
          printf("subnet mask not found\n");
        } else {
          printf("subnet mask: %s\n", ip4addr_ntoa((ip4_addr_t*)&obj->data.u32));
        }

        res = db_get_object(&iface->objects, DB_ID_IP_GATEWAY, &obj);
        if (res != SPN_OK) {
          printf("gateway not found\n");
        } else {
          printf("gateway: %s\n", ip4addr_ntoa((ip4_addr_t*)&obj->data.u32));
        }
      }
      break;
    case APP_STATE_PRM:
      printf("prm state\n");
      break;
    case APP_STATE_RUN:
      printf("run state\n");
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
  sys_timeout(1000, app_rta_timer_handler, &app_inst);
  return 0;
}
