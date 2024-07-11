#include "lwip/def.h"
#include "spn/spn_sys.h"
#include <spn/dcp.h>
#include <string.h>

extern uint16_t* spn_dcp_supported_options;

/* All use spn_dcp_pack.c's impl */

void spn_dcp_pack_block(void* dest, uint16_t option_sub_option, uint16_t payload_len, uint16_t block_info)
{
    struct spn_dcp_general_block* block = (struct spn_dcp_general_block*)dest;
    uint16_t* block_info_ptr = (uint16_t*)(block + 1);
    block->option = (option_sub_option >> 8);
    block->sub_option = option_sub_option & 0xFF;
    block->dcp_block_length = lwip_htons(payload_len + 2);
    *block_info_ptr = lwip_htons(block_info);
}

void spn_dcp_pack_ip(void* dest, iface_t* iface)
{
    uint32_t* ip = (uint32_t*)dest;
    uint32_t* mask = ip + 1;
    uint32_t* gw = mask + 1;

    *ip = spn_sys_get_ip_addr(iface);
    *mask = spn_sys_get_ip_mask(iface);
    *gw = spn_sys_get_ip_gw(iface);
}

void spn_dcp_pack_dns(void* dest, iface_t* iface)
{
    uint32_t* dns = (uint32_t*)dest;
    int idx = 0;
    for (idx = 0; idx < 4; idx++) {
        dns[idx] = spn_sys_get_dns(iface, idx);
    }
}

int spn_dcp_pack_station_of_name(void* dest)
{
    const char* station_name = spn_sys_get_station_name();
    int len;
    if (station_name == NULL) {
        return -1;
    }
    len = strlen(station_name);
    memcpy(dest, station_name, len);
    return len;
}

int spn_dcp_pack_alias(void* dest)
{
    const char* alias_name = spn_sys_get_alias_name();
    int len;
    if (alias_name == NULL) {
        return -1;
    }
    len = strlen(alias_name);
    memcpy(dest, alias_name, len);
    return len;
}

int spn_dcp_pack_vendor_name(void* dest)
{
    const char* vendor_name = spn_sys_get_vendor_name();
    int len;
    if (vendor_name == NULL) {
        return -1;
    }
    len = strlen(vendor_name);
    memcpy(dest, vendor_name, len);
    return len;
}

void spn_dcp_pack_device_id(void* dest)
{
    uint16_t* vendor_id = (uint16_t*)dest;
    uint16_t* device_id = vendor_id + 1;
    *vendor_id = spn_sys_get_vendor_id();
    *device_id = spn_sys_get_device_id();
}

void spn_dcp_pack_oem_id(void* dest)
{
    uint16_t* vendor_id = (uint16_t*)dest;
    uint16_t* device_id = vendor_id + 1;
    *vendor_id = spn_sys_get_oem_vendor_id();
    *device_id = spn_sys_get_oem_device_id();
}

void spn_dcp_pack_role(void* dest)
{
    enum spn_role role = spn_sys_get_role();
    uint8_t* role_ptr = (uint8_t*)dest;

    *role_ptr = (uint8_t)role;
    *(role_ptr + 1) = 0;
}

int spn_dcp_pack_options(void* dest)
{
    int len = 0;
    uint16_t* option = (uint16_t*)dest;
    const uint16_t* r_option = spn_dcp_supported_options;

    while (*r_option) {
        *option++ = lwip_htons(*r_option++);
        len++;
    }
    return len;
}

void spn_dcp_pack_instance(void* dest)
{
    uint16_t* instance = (uint16_t*)dest;
    *instance = lwip_htons(1); /* TODO: get vars from sm db */
}

void spn_dcp_pack_device_initiative(void* dest)
{
    uint16_t* value = (uint16_t*)dest;
    *value = lwip_htons(SPN_DCP_DEV_INITIATIVE_ENABLE_HELLO); /* TODO: get vars from sm db */
}
