#include "lwip/def.h"
#include "spn/spn_sys.h"
#include <spn/dcp.h>
#include <string.h>

/* All use spn_dcp_pack.c's impl */

void spn_dcp_pack_resp_block(void* dest, uint16_t option_sub_option, uint16_t payload_len, uint16_t block_info)
{
    struct spn_dcp_general_block* block = (struct spn_dcp_general_block*)dest;
    uint16_t* block_info_ptr = (uint16_t*)((uintptr_t)(block + 1) & ~1);
    block->option = (option_sub_option >> 8);
    block->sub_option = option_sub_option & 0xFF;
    block->dcp_block_length = lwip_ntohs(payload_len + 2);
    *block_info_ptr = lwip_ntohs(block_info);
}

void spn_dcp_pack_ip(void* dest, uint32_t ip, uint32_t mask, uint32_t gw)
{
    uint32_t* _ip = (uint32_t*)dest;
    uint32_t* _mask = _ip + 1;
    uint32_t* _gw = _mask + 1;

    *_ip = ip;
    *_mask = mask;
    *_gw = gw;
}

void spn_dcp_pack_dns(void* dest, uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4)
{
    uint32_t* dns = (uint32_t*)dest;
    dns[0] = dns1;
    dns[1] = dns2;
    dns[2] = dns3;
    dns[3] = dns4;
}

int spn_dcp_pack_station_of_name(void* dest, const char* name)
{
    const char* station_name = name;
    int len;
    if (station_name == NULL) {
        return -1;
    }
    len = strlen(station_name);
    memcpy(dest, station_name, len);
    return len;
}

int spn_dcp_pack_alias(void* dest, const char* name)
{
    const char* alias_name = name;
    int len;
    if (alias_name == NULL) {
        return -1;
    }
    len = strlen(alias_name);
    memcpy(dest, alias_name, len);
    return len;
}

int spn_dcp_pack_vendor_name(void* dest, const char* name)
{
    const char* vendor_name = name;
    int len;
    if (vendor_name == NULL) {
        return -1;
    }
    len = strlen(vendor_name);
    memcpy(dest, vendor_name, len);
    return len;
}

void spn_dcp_pack_device_id(void* dest, uint16_t vendor_id, uint16_t device_id)
{
    uint16_t* p_vendor_id = (uint16_t*)dest;
    uint16_t* p_device_id = p_vendor_id + 1;
    *p_vendor_id = vendor_id;
    *p_device_id = device_id;
}

void spn_dcp_pack_oem_id(void* dest, uint16_t vendor_id, uint16_t device_id)
{
    uint16_t* p_vendor_id = (uint16_t*)dest;
    uint16_t* p_device_id = p_vendor_id + 1;
    *p_vendor_id = vendor_id;
    *p_device_id = device_id;
}

void spn_dcp_pack_role(void* dest, enum spn_role role)
{
    uint8_t* role_ptr = (uint8_t*)dest;

    *role_ptr = (uint8_t)role;
    *(role_ptr + 1) = 0;
}

int spn_dcp_pack_options(void* dest, const uint16_t* options)
{
    int len = 0;
    uint16_t* option = (uint16_t*)dest;
    const uint16_t* r_option = options;

    while (*r_option) {
        *option++ = lwip_ntohs(*r_option++);
        len++;
    }
    return len;
}

void spn_dcp_pack_instance(void* dest, uint16_t instance)
{
    uint16_t* p_instance = (uint16_t*)dest;
    *p_instance = instance;
}

void spn_dcp_pack_device_initiative(void* dest, uint16_t initiative)
{
    uint16_t* value = (uint16_t*)dest;
    *value = initiative;
}
