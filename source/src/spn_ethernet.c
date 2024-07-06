#include "spn_iface.h"
#include <spn/arch.h>
#include <spn/errno.h>
#include <spn/ethernet.h>
#include <spn/ip.h>
#include <spn/pn_rtc.h>
#include <string.h>

static inline int is_multicast_mac(const char* mac)
{
    return (mac[0] & 0x01);
}

static inline int mac_cmp(const char* mac1, const char* mac2)
{
    return SPN_IFACE_MAC_H(mac1) == SPN_IFACE_MAC_H(mac2) && SPN_IFACE_MEM_L(mac1) == SPN_IFACE_MEM_L(mac2);
}

int spn_eth_input(spn_iface_t* iface, void* ethernet_frame, size_t len)
{
    const spn_ethernet_t* header = ethernet_frame;
    const uint16_t ethertype = SPN_ETH_TYPE(header);
    const char* dst_mac = SPN_ETH_DST_MAC(header);
    int res;

    switch (SPN_HTONS(ethertype)) {
    case SPN_ETH_TYPE_IP:
        if (unlikely(!mac_cmp(dst_mac, (char*)iface->mac))) {
            res = -SPN_EAGAIN;
            break;
        }
        res = spn_ip_input((void*)SPN_ETH_PAYLOAD(header), len - sizeof(spn_ethernet_t));
    case SPN_ETH_TYPE_PN:
        if (mac_cmp(dst_mac, (char*)iface->mac) || is_multicast_mac(SPN_ETH_DST_MAC(header))) {
            // res = spn_pn_rtc_input(SPN_ETH_PAYLOAD(header), len - sizeof(spn_ethernet_t));
        } else {
            res = -SPN_EAGAIN;
        }
        break;
    case SPN_ETH_TYPE_VLAN:
    default:
        // NOTE: Unknow frame type, drop to others to handle it
        res = -SPN_EAGAIN;
    }
    return res;
}

int spn_eth_output(spn_iface_t* iface, const void* ethernet_frame, size_t len)
{
    const spn_ethernet_t* header = ethernet_frame;
    const uint16_t ethertype = SPN_ETH_TYPE(header);
    bool xfer_res;

    switch (SPN_HTONS(ethertype)) {
    case SPN_ETH_TYPE_PN:
        xfer_res = spn_iface_output_poll(iface->port, ethernet_frame, len, SPN_IFACE_REALTIME_TIMEOUT);
        break;
    case SPN_ETH_TYPE_IP:
    case SPN_ETH_TYPE_VLAN:
    default:
        // NOTE: Assume unknown frame is not a real-time frame
        xfer_res = spn_iface_output(iface->port, ethernet_frame, len);
        break;
    }

    if (xfer_res) {
        return SPN_OK;
    }
    return -SPN_EIO;
}
