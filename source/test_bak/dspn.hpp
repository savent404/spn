#pragma once

#include <gtest/gtest.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <functional>

namespace test {
struct Dspn {
  using port_init_t = std::function<int(struct spn_ctx*, spn_iface_t*, int, int)>;
  using dcp_output_t = std::function<int(struct dcp_ctx*, struct spn_iface*, const struct eth_addr*, struct pbuf*)>;

  static Dspn* get_instance() {
    static Dspn instance;
    return &instance;
  }

  port_init_t port_init;
  dcp_output_t dcp_output;
};
};  // namespace test
