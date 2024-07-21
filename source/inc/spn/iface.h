#pragma once

#include <lwip/netif.h>
#include <stdint.h>

typedef struct spn_iface {
  struct netif netif;
} spn_iface_t;
