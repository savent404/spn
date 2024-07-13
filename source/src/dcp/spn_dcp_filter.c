#include <lwip/arch.h>
#include <lwip/def.h>
#include <spn/dcp.h>
#include <spn/spn_sys.h>
#include <string.h>

bool spn_dcp_filter_ip(iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw)
{
    uint32_t t;

    t = lwip_ntohl(spn_sys_get_ip_addr(iface));
    if (t != ip) {
        return false;
    }

    t = lwip_ntohl(spn_sys_get_ip_mask(iface));
    if (t != mask && mask != 0) {
        return false;
    }

    t = lwip_ntohl(spn_sys_get_ip_gw(iface));
    if (t != gw && gw != 0) {
        return false;
    }
    return true;
}

bool spn_dcp_filter_dns(iface_t* iface, uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4)
{
    uint32_t t;

    t = lwip_ntohl(spn_sys_get_dns(iface, 0));
    if (t != dns1 && dns1 != 0) {
        return false;
    }

    t = lwip_ntohl(spn_sys_get_dns(iface, 1));
    if (t != dns2 && dns2 != 0) {
        return false;
    }

    t = lwip_ntohl(spn_sys_get_dns(iface, 2));
    if (t != dns3 && dns3 != 0) {
        return false;
    }

    t = lwip_ntohl(spn_sys_get_dns(iface, 3));
    if (t != dns4 && dns4 != 0) {
        return false;
    }

    return true;
}

bool spn_dcp_filter_station_of_name(const char* name, uint16_t len)
{
    const char* t;

    t = spn_sys_get_station_name();
    if (!t || strncmp(t, name, len) != 0) {
        return false;
    }
    return true;
}

bool spn_dcp_filter_alias(const char* name, uint16_t len)
{
    /* Code of alias name: NameOfPort.NameOfStation */
    int dotIdx;
    const char* t;

    if (!name || !len) {
        return false;
    }

    /* Find dot idx */
    for (dotIdx = 0; dotIdx < len; dotIdx++) {
        if (name[dotIdx] == '.') {
            break;
        }
    }
    if (dotIdx == len) {
        return false;
    }

    t = spn_sys_get_alias_name();
    if (!t || strncmp(name + dotIdx, t, len - dotIdx) != 0) {
        return false;
    }
    /* TODO: Check a reset of NameOfPort is valid */
    return true;
}

bool spn_dcp_filter_vendor_name(const char* name, uint16_t len)
{
    const char* t;

    t = spn_sys_get_vendor_name();
    if (!t || strncmp(t, name, len) != 0) {
        return false;
    }

    if (strncmp(t, name, len)) {
        return false;
    }

    return true;
}

bool spn_dcp_filter_vendor_id(uint16_t vendor_id, uint16_t device_id)
{
    if (vendor_id != spn_sys_get_vendor_id() || device_id != spn_sys_get_device_id()) {
        return false;
    }
    return true;
}

bool spn_dcp_filter_oem_id(uint16_t vendor_id, uint16_t device_id)
{
    if (vendor_id != spn_sys_get_oem_vendor_id() || device_id != spn_sys_get_oem_device_id()) {
        return false;
    }
    return true;
}

bool spn_dcp_filter_role(uint8_t role)
{
    if (role != spn_sys_get_role()) {
        return false;
    }
    return true;
}

extern const uint16_t* spn_dcp_supported_options;
static inline bool option_is_supported(uint16_t option)
{
    const uint16_t* p = spn_dcp_supported_options;
    while (*p) {
        if (*p == option) {
            return true;
        }
        p++;
    }
    return false;
}

bool spn_dcp_filter_options(const uint16_t* options, uint16_t num)
{
    uint16_t i;
    for (i = 0; i < num; i++) {
        if (!option_is_supported(options[i])) {
            return false;
        }
    }
    return true;
}

bool spn_dcp_filter_instance(uint16_t instance)
{
    LWIP_UNUSED_ARG(instance);
    /* TODO: Implement this function */
    return true;
}
bool spn_dcp_filter_device_initiative(uint16_t value)
{
    LWIP_UNUSED_ARG(value);
    /* TODO: Implement this function */
    return true;
}
