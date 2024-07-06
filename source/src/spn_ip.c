#include <spn.h>
#include <spn/arch.h>
#include <spn/errno.h>
#include <spn/ethernet.h>
#include <spn/ip.h>
#include <spn_iface.h>
#include <string.h>

static inline int ip_cmp(const char* ip1, const char* ip2)
{
    return SPN_IFACE_IP(ip1) == SPN_IFACE_IP(ip2);
}

int spn_ip_input(void* ip_frame, size_t len)
{
    ip_hdr_t* ip_hdr = (ip_hdr_t*)ip_frame;

    switch (ip_hdr->proto) {
    case SPN_IP_PROTO_UDP:
        // UDP
        return SPN_OK;
    case SPN_IP_PROTO_TCP:
    case SPN_IP_PROTO_ICMP:
    default:
        break;
    }
    return -SPN_EAGAIN;
}

int spn_ip_output(void* ip_frame, spn_ip_addr_t addr, size_t len)
{
    ip_hdr_t* ip_hdr = (ip_hdr_t*)ip_frame;
    spn_ethernet_t* eth_hdr = (spn_ethernet_t*)((char*)ip_frame - sizeof(spn_ethernet_t));
    spn_instance_t* inst = spn_get_inst();
    spn_iface_t* iface = NULL;

    switch (ip_hdr->proto) {
    case SPN_IP_PROTO_UDP:
        break;
    case SPN_IP_PROTO_TCP:
    case SPN_IP_PROTO_ICMP:
    default:
        return -SPN_EINVAL;
    }

    // Find the iface
    for (int i = 0; i < SPN_IFACE_MAX_NUM; i++) {
        if (SPN_IFACE_IP(inst->ifaces[i].ip) == ip_hdr->saddr) {
            iface = &inst->ifaces[i];
            eth_hdr->ethertype = SPN_HTONS(SPN_ETH_TYPE_IP);
            memcpy(eth_hdr->src_mac, iface->mac, 6);
        }
    }
}