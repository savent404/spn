#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void dft_port_init(struct netif* iface, const char* port_name, const uint32_t ip);
void dft_port_deinit(struct netif* iface);

#ifdef __cplusplus
}
#endif
