#pragma once

#include <lwip/netif.h>

typedef struct iface {
  struct netif netif;
} iface_t;
