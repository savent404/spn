#pragma once

#include <spn/iface.h>
#include <list>
#include <memory>

struct iface_instace {
  struct spn_priv_frame {
    char buff[1516];
    char dst[6];
    uint16_t len;
    spn_ftype_t type;
    spn_iface_t* iface;
  };
  using frame_ptr_t = spn_priv_frame*;

  std::list<frame_ptr_t> fifo;

  static iface_instace* get_instance() {
    static iface_instace instance;
    return &instance;
  }
};
