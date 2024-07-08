#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void default_netif_init(struct netif* ifaces, const char *port1_name, const char *port2_name, const uint32_t ip);

#ifdef __cplusplus
}
#endif
