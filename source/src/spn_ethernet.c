#include "spn/arch.h"
#include "spn/errno.h"
#include "spn/ethernet.h"
#include "spn_iface.h"

int spn_eth_input(spn_iface_t* iface, void* ethernet_frame, size_t len)
{
    const spn_ethernet_t* header = ethernet_frame;
    const uint16_t ethertype = SPN_ETH_TYPE(header);

    switch (SPN_HTONS(ethertype)) {
    case SPN_ETH_TYPE_IP:
        // IP packet
        break;
    case SPN_ETH_TYPE_VLAN:
        // VLAN packet
        break;
    case SPN_ETH_TYPE_PN:
        // PN packet
        break;
    default:
        // NOTE: Unknow frame type, drop to others to handle it
        return -SPN_AGAIN;
    }
    return SPN_OK;
}

void spn_eth_output(spn_iface_t* iface, const void* ethernet_frame, size_t len)
{
    const spn_ethernet_t* header = ethernet_frame;
    const uint16_t ethertype = SPN_ETH_TYPE(header);

    switch (SPN_HTONS(ethertype)) {
    case SPN_ETH_TYPE_PN:
        spn_iface_output_poll(iface->port, ethernet_frame, len, SPN_IFACE_REALTIME_TIMEOUT);
        break;
    case SPN_ETH_TYPE_IP:
    case SPN_ETH_TYPE_VLAN:
    default:
        // NOTE: Assume unknown frame is not a real-time frame
        spn_iface_output(iface->port, ethernet_frame, len);
        break;
    }
}
