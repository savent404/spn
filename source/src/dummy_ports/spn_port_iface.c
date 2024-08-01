#include <spn/iface.h>
#include <spn/sys.h>

__attribute__((weak)) int spn_iface_set_addr(spn_iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw) {
  SPN_UNUSED_ARG(iface);
  SPN_UNUSED_ARG(ip);
  SPN_UNUSED_ARG(mask);
  SPN_UNUSED_ARG(gw);
  return 0;
}
