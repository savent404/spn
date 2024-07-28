#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <spn/spn.h>
#include <string.h>
#include "ddcp.hpp"
#include "dspn.hpp"
#include "test_data.hpp"

using namespace test_data::dcp;

namespace {

static void lwip_global_init(void) {
  static bool initialized = false;
  if (!initialized) {
    lwip_init();
    initialized = true;
  }
}

struct FDcp : public ::testing::Test {
  void SetUp() override {
    struct db_interface* interface;
    struct db_port* port;
    db_value_t val;
    lwip_global_init();

    auto inst = test::Dspn::get_instance();
    inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return SPN_OK; };
    inst->dcp_output = [](struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* dst, struct pbuf* p) { return 0; };

    spn_init(&ctx, &cfg);

    /* default setup */
    auto& db = ctx.db;
    db_get_interface(&db, 0, &interface);
    db_get_port(interface, 0, &port);
  }
  void TearDown() override { spn_deinit(&ctx); }
  struct spn_ctx ctx;
  struct spn_cfg cfg = {
    .vendor_name = "test",
  };
};
}  // namespace

TEST_F(FDcp, ident_req) {
  DataParser parser;
  auto f = parser(kDcpAllSelector);
  struct pbuf* p = pbuf_alloc(PBUF_RAW, f->size(), PBUF_RAM);
  memcpy(p->payload, f->data(), f->size());
  EXPECT_EQ(spn_input_hook(p, &ctx.ifaces[0][0]), 0);
  pbuf_free(p);
}

TEST_F(FDcp, ident_req_delayed) {
  DataParser parser;
  auto f = parser(kDcpAllSelector);
  f->at(22) = 0x00;
  f->at(23) = 0x02;
  struct pbuf* p = pbuf_alloc(PBUF_RAW, f->size(), PBUF_RAM);
  memcpy(p->payload, f->data(), f->size());

  EXPECT_EQ(spn_input_hook(p, &ctx.ifaces[0][0]), 0);

  sys_check_timeouts();

  pbuf_free(p);
}
