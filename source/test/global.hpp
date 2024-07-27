#pragma once

#include <assert.h>
#include <lwip/etharp.h>
#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/pbuf.h>
#include <lwip/timeouts.h>
#include <lwip/udp.h>

struct LwipCtx {
  enum class level : int {
    none = 0,
    core,
    mem,
    netif,
    timer,
    full,
    invalid,
  };

  LwipCtx(level lvl) {
    while (lvl > init_lvl && init_lvl != level::invalid) {
      init_lvl = do_init(level(int(init_lvl) + 1));
    }
  }

  ~LwipCtx() {}

  level do_init(level lvl) {
    switch (lvl) {
      case level::core:
        sys_init();
        return level::core;
      case level::mem:
        mem_init();
        memp_init();
        pbuf_init();
        return level::mem;
      case level::netif:
        netif_init();
        return level::netif;
      case level::timer:
        sys_timeouts_init();
        return level::timer;
      case level::full:
        ip_init();
        etharp_init();
        udp_init();
        return level::full;
      default:
        return level::invalid;
    }
  }

  static LwipCtx* get_instance(level lvl = level::mem) {
    if (!instance) {
      instance = new LwipCtx(level(lvl));
      instance->init_lvl = lvl;
    }

    auto inst = instance;

    while (lvl > inst->init_lvl && inst->init_lvl != level::invalid) {
      inst->init_lvl = inst->do_init(level(int(inst->init_lvl) + 1));
    }
    assert(inst->init_lvl != level::invalid);
    return inst;
  }

  void cleanup() {
    if (instance) {
      delete instance;
      instance = nullptr;
    }
  }

  LwipCtx(const LwipCtx&) = delete;
  static LwipCtx* instance;
  static level init_lvl;
};