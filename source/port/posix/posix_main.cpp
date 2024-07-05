#include "spn_iface.h"
#include <cstdlib>
#include <cstring>
#include <spn/arch.h>
#include <spn/ethernet.h>

int main(int argc, char* argv[])
{
    spn_iface_t _iface = {
        .port = 0,
    };
    auto iface = &_iface;
    spn_iface_get_ip_address(iface->port, iface->ip);
    spn_iface_get_mac_address(iface->port, iface->mac);

    char frame_buffer[1500];
    spn_ethernet_t* f = (spn_ethernet_t*)frame_buffer;

    memset(frame_buffer, 0, sizeof(frame_buffer));
    memset(SPN_ETH_DST_MAC(f), 0xFF, 6);
    memcpy(SPN_ETH_SRC_MAC(f), iface->mac, 6);
    SPN_ETH_TYPE(f) = SPN_NTOHS(SPN_ETH_TYPE_PN);
    std::strcpy((char*)SPN_ETH_PAYLOAD(f), "Hello,World");
    spn_eth_output(iface, f, 1500);
    return 0;
}
