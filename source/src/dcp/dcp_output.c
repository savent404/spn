#include <spn/dcp.h>
#include <spn/iface.h>
#include <spn/sys.h>

__attribute__((weak)) int dcp_output(struct dcp_ctx* ctx,
                                     struct spn_iface* iface,
                                     const struct eth_addr* dst,
                                     spn_frame_t f) {
  SPN_UNUSED_ARG(ctx);
  /* TODO: should call generic ethernet_output of spn */
  const struct eth_addr* src = (const struct eth_addr*)&iface->netif.hwaddr;
  spn_send_frame(iface, f, (const uint8_t*)dst);
  return 0;
}
