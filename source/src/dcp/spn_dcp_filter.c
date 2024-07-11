#include "lwip/arch.h"
#include <spn/dcp.h>

bool spn_dcp_filter_ip(uint32_t ip, uint32_t mask, uint32_t gw)
{
    LWIP_UNUSED_ARG(ip);
    LWIP_UNUSED_ARG(mask);
    LWIP_UNUSED_ARG(gw);
    return true;
}
bool spn_dcp_filter_dns(uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4)
{
    LWIP_UNUSED_ARG(dns1);
    LWIP_UNUSED_ARG(dns2);
    LWIP_UNUSED_ARG(dns3);
    LWIP_UNUSED_ARG(dns4);
    return true;
}
bool spn_dcp_filter_station_of_name(const char* name, uint16_t len)
{
    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(len);
    return true;
}
bool spn_dcp_filter_alias(const char* name, uint16_t len)
{
    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(len);
    return true;
}
bool spn_dcp_filter_vendor_name(const char* name, uint16_t len)
{
    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(len);
    return true;
}
bool spn_dcp_filter_vendor_id(uint16_t vendor_id, uint16_t device_id)
{
    LWIP_UNUSED_ARG(vendor_id);
    LWIP_UNUSED_ARG(device_id);
    return true;
}
bool spn_dcp_filter_oem_id(uint16_t vendor_id, uint16_t device_id)
{
    LWIP_UNUSED_ARG(vendor_id);
    LWIP_UNUSED_ARG(device_id);
    return true;
}
bool spn_dcp_filter_role(uint8_t role)
{
    LWIP_UNUSED_ARG(role);
    return true;
}
bool spn_dcp_filter_options(const uint16_t* options, uint16_t num)
{
    LWIP_UNUSED_ARG(options);
    LWIP_UNUSED_ARG(num);
    return true;
}
bool spn_dcp_filter_instance(uint16_t instance)
{
    LWIP_UNUSED_ARG(instance);
    return true;
}
bool spn_dcp_filter_device_initiative(uint16_t value)
{
    LWIP_UNUSED_ARG(value);
    return true;
}
