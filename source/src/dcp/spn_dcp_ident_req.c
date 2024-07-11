#include <spn/dcp.h>
#include <spn/errno.h>

int spn_dcp_ident_req_parse(void* payload, uint16_t len, struct spn_dcp_ident_req* reqs)
{
    const struct spn_dcp_general_block* block_ptr;
    uint16_t offset;
    uint16_t block_len;
    uint16_t block_type;
    const char* r_payload;
    unsigned int idx;

    /* temps for filter */
    uint32_t ip_addr, ip_mask, ip_gw, ip_dns[4];
    uint16_t name_len, vendor_id, device_id, tmp_u16;
    const char* name;

    idx = 0;
    r_payload = (const char*)payload;

    for (offset = 0; offset < len;) {
        LWIP_ASSERT("DCP: At least 16bit align", (offset & 2) == 0);
        r_payload = (const char*)((uintptr_t)payload + offset);
        block_ptr = (const struct spn_dcp_general_block*)r_payload;
        block_len = lwip_htons(block_ptr->dcp_block_length);
        block_type = BLOCK_TYPE(block_ptr->option, block_ptr->sub_option);

        if (block_len + sizeof(*block_ptr) + offset > len) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_HALT, ("DCP: invalid block_len=%d, len=%d, offset=%d\n", block_len, len, offset));
            return -SPN_EBADMSG;
        }

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            LWIP_ASSERT("Invalid block length", block_len == 14);
            ip_addr = lwip_htonl(GET_VALUE(r_payload, uint32_t, 0));
            ip_mask = lwip_htonl(GET_VALUE(r_payload, uint32_t, 4));
            ip_gw = lwip_htonl(GET_VALUE(r_payload, uint32_t, 8));
            if (spn_dcp_filter_ip(ip_addr, ip_mask, ip_gw)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            LWIP_ASSERT("Invalid block length", block_len == 30);
            ip_addr = lwip_htonl(GET_VALUE(r_payload, uint32_t, 0));
            ip_mask = lwip_htonl(GET_VALUE(r_payload, uint32_t, 4));
            ip_gw = lwip_htonl(GET_VALUE(r_payload, uint32_t, 8));
            ip_dns[0] = lwip_htons(GET_VALUE(r_payload, uint32_t, 12));
            ip_dns[1] = lwip_htons(GET_VALUE(r_payload, uint32_t, 16));
            ip_dns[2] = lwip_htons(GET_VALUE(r_payload, uint32_t, 20));
            ip_dns[3] = lwip_htons(GET_VALUE(r_payload, uint32_t, 24));
            if (spn_dcp_filter_ip(ip_addr, ip_mask, ip_gw)) {
                goto filter_miss_match;
            }
            if (spn_dcp_filter_dns(ip_dns[0], ip_dns[1], ip_dns[2], ip_dns[3])) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            name = r_payload;
            name_len = block_len;
            if (spn_dcp_filter_vendor_name(name, name_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            name = r_payload;
            name_len = block_len;
            if (spn_dcp_filter_station_of_name(name, name_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            LWIP_ASSERT("Invalid block length", block_len == 4);
            vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            if (spn_dcp_filter_vendor_id(vendor_id, device_id)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            if (spn_dcp_filter_role(GET_VALUE(r_payload, uint8_t, 0))) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            LWIP_ASSERT("Invalid block length", block_len >= 2);
            if (spn_dcp_filter_options((const uint16_t*)r_payload, block_len / 2)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            if (spn_dcp_filter_alias(r_payload, block_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            tmp_u16 = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            if (spn_dcp_filter_instance(tmp_u16)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            LWIP_ASSERT("Invalid block length", block_len == 4);
            vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            if (spn_dcp_filter_oem_id(vendor_id, device_id)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            return -SPN_ENOSYS;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            tmp_u16 = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            if (spn_dcp_filter_device_initiative(tmp_u16)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
            return -SPN_ENOSYS;
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            break;
        default:
            goto filter_miss_match;
        }

        /* Push option into reqs */
        if (idx >= sizeof(reqs->option_sub_option) / sizeof(reqs->option_sub_option[0])) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP.ident_req_parse: too many blocks\n"));
            return -SPN_EMSGSIZE;
        }
        reqs->option_sub_option[idx++] = block_type;

        /* Goto next req block */
        offset = spn_dcp_block_walk(payload, offset);
    }

    return SPN_OK;
filter_miss_match:
    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP.ident_req_parse: filter miss match, last block_type=0x%04x\n", block_type));
    return -SPN_ENXIO;
}
