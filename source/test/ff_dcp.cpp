#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <spn/spn.h>
#include <string.h>
#include "ddcp.hpp"
#include "dspn.hpp"
#include "test_data.hpp"

using namespace test_data::dcp;

static void lwip_global_init(void) {
  static bool initialized = false;
  if (!initialized) {
    lwip_init();
    initialized = true;
  }
}

TEST(Fdcp, ident_req) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return SPN_OK; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  // TODO: add db objects
  lwip_global_init();

  DataParser parser;
  auto f = parser(kDcpAllSelector);
  struct pbuf* p = pbuf_alloc(PBUF_RAW, f->size(), PBUF_RAM);
  memcpy(p->payload, f->data(), f->size());

  EXPECT_EQ(spn_init(&ctx, &cfg), SPN_OK);
  EXPECT_EQ(spn_input_hook(p, &ctx.ifaces[0][0]), 0);

  // TODO: check the output

  pbuf_free(p);

  spn_deinit(&ctx);
}

TEST(Fdcp, ident_req_delayed) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return SPN_OK; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  // TODO: add db objects
  lwip_global_init();

  DataParser parser;
  auto f = parser(kDcpAllSelector);
  f->at(22) = 0x00;
  f->at(23) = 0x02;
  struct pbuf* p = pbuf_alloc(PBUF_RAW, f->size(), PBUF_RAM);
  memcpy(p->payload, f->data(), f->size());

  EXPECT_EQ(spn_init(&ctx, &cfg), SPN_OK);
  EXPECT_EQ(spn_input_hook(p, &ctx.ifaces[0][0]), 0);

  sys_check_timeouts();

  // TODO: check the output

  pbuf_free(p);
  spn_deinit(&ctx);
}
