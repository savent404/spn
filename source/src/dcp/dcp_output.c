#include <lwip/netif.h>
#include <spn/dcp.h>
#include <spn/sys.h>

__attribute__((weak)) int dcp_output(struct dcp_ctx* ctx,
                                     struct spn_iface* iface,
                                     const struct eth_addr* dst,
                                     struct pbuf* p) {
  SPN_UNUSED_ARG(ctx);
  /* TODO: should call generic ethernet_output of spn */
  const struct eth_addr* src = (const struct eth_addr*)&iface->netif.hwaddr;
  ethernet_output(&iface->netif, p, src, dst, ETHTYPE_PROFINET);
  return 0;
}
