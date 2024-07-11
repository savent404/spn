#include <lwip/dns.h>
#include <spn/spn_sys.h>
#include <stdlib.h>
#include <string.h>

SPN_WEAK_FN void spn_sys_init(void)
{
}

SPN_WEAK_FN uint32_t spn_sys_get_ip_addr(iface_t* iface)
{
    uint32_t t;
    memcpy(&t, &iface->ip_addr.addr, sizeof(t));
    return lwip_htons(t);
}

SPN_WEAK_FN void spn_sys_get_mac_addr(iface_t* iface, char* mac)
{
    memcpy(mac, iface->hwaddr, 6);
}

SPN_WEAK_FN uint32_t spn_sys_get_ip_mask(iface_t* iface)
{
    uint32_t t;
    memcpy(&t, &iface->netmask.addr, sizeof(t));
    return lwip_htons(t);
}

SPN_WEAK_FN uint32_t spn_sys_get_ip_gw(iface_t* iface)
{
    uint32_t t;
    memcpy(&t, &iface->gw.addr, sizeof(t));
    return lwip_htons(t);
}

SPN_WEAK_FN uint32_t spn_sys_get_dns(iface_t* iface, int idx)
{
    LWIP_UNUSED_ARG(iface);
#if LWIP_DNS
    return lwip_htonl(dns_getserver(idx));
#else
    LWIP_UNUSED_ARG(idx);
    return 0;
#endif
}

SPN_WEAK_FN const char* spn_sys_get_station_name(void)
{
    return "SPN_001";
}

SPN_WEAK_FN const char* spn_sys_get_alias_name(void)
{
    return "SPN_001.00";
}

SPN_WEAK_FN const char* spn_sys_get_vendor_name(void)
{
    return "SPN ShiYuShu Inc.";
}

SPN_WEAK_FN uint16_t spn_sys_get_vendor_id(void)
{
    return PP_NTOHS(0x1234);
}

SPN_WEAK_FN uint16_t spn_sys_get_device_id(void)
{
    return PP_NTOHS(0x5678);
}

SPN_WEAK_FN uint16_t spn_sys_get_oem_vendor_id(void)
{
    return PP_NTOHS(0xBEEF);
}

SPN_WEAK_FN uint16_t spn_sys_get_oem_device_id(void)
{
    return PP_NTOHS(0x1234);
}

SPN_WEAK_FN enum spn_role spn_sys_get_role(void)
{
    return role_iod;
}

SPN_WEAK_FN enum spn_ip_status spn_sys_get_ip_status(void)
{
    return ip_status_static;
}
