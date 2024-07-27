#include <assert.h>
#include <lwip/timeouts.h>
#include <spn/dcp.h>
#include <spn/spn.h>
#include <stdio.h>

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
  struct app_instance* inst = &app_inst;
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
      APP_STATE_DCP;
      break;
    case APP_STATE_DCP:
      printf("dcp state\n");

      // make sure that ident.req context is cleared, and device should be located in the db
      assert(inst->ctx->dcp.mcs_ctx.resp_options_bitmap == 0);
      assert(inst->ctx->dcp.mcs_ctx.state == DCP_STATE_IDLE);
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
