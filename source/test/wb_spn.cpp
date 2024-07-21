#include <gtest/gtest.h>
#include <spn/spn.h>
#include "dspn.hpp"

extern "C" int spn_port_init(struct spn_ctx* ctx, struct spn_iface* iface, uint16_t interface, uint16_t port) {
  auto inst = test::Dspn::get_instance();
  return inst->port_init(ctx, iface, interface, port);
}

TEST(Dspn, init) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return SPN_OK; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  EXPECT_EQ(spn_init(&ctx, &cfg), SPN_OK);
  spn_deinit(&ctx);
}

TEST(Dspn, warning_init) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return -SPN_ENOENT; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  EXPECT_EQ(spn_init(&ctx, &cfg), -SPN_ENOENT);

  // Hope memleak not happen
  spn_deinit(&ctx);
}


TEST(Dspn, fatal_init) {
  auto inst = test::Dspn::get_instance();

  inst->port_init = [](struct spn_ctx* ctx, spn_iface_t* iface, int interface, int port) { return -SPN_EINVAL; };

  struct spn_ctx ctx;
  struct spn_cfg cfg = {};
  int err;

  EXPECT_EQ(spn_init(&ctx, &cfg), -SPN_EINVAL);

  // Hope memleak not happen
  spn_deinit(&ctx);
}
