#include <spn/iface.h>

__attribute__((weak)) int spn_iface_set_addr(spn_iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw) {
  return 0;
}
