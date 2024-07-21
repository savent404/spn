#pragma once

#include <gtest/gtest.h>
#include <spn/spn.h>
#include <functional>

namespace test {
struct Dspn {
  using port_init_t = std::function<int(struct spn_ctx*, spn_iface_t*, int, int)>;

  static Dspn* get_instance() {
    static Dspn instance;
    return &instance;
  }

  port_init_t port_init;
};
};  // namespace test
