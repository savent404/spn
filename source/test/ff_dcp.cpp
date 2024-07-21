#include <lwip/init.h>
#include <spn/spn.h>
#include <string.h>
#include "ddcp.hpp"
#include "dspn.hpp"
#include "test_data.hpp"

using namespace test_data::dcp;

TEST(Fdcp, ident_req) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return SPN_OK; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  lwip_init();

  DataParser parser;
  auto f = parser(kDcpAllSelector);
  struct pbuf* p = pbuf_alloc(PBUF_RAW, f->size(), PBUF_RAM);
  memcpy(p->payload, f->data(), f->size());

  EXPECT_EQ(spn_init(&ctx, &cfg), SPN_OK);
  EXPECT_EQ(spn_input_hook(p, &ctx.ifaces[0][0]), 0);

  spn_deinit(&ctx);
}
