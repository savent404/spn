#include "lwip/debug.h"
#include <lwip/opt.h>
#include <netif/ethernet.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/spn_sys.h>
#include <string.h>

extern const uint16_t* spn_dcp_mandatory_reqs;
extern const uint16_t* spn_dcp_supported_options;

/**
 * @brief Merge to line into line
 *
 * @param[out] dest
 * @param[in] s1
 * @param[in] s2
 * @param max_len dest max length
 * @return int
 */
static inline int spn_dcp_merge_options(uint16_t* dest, const uint16_t* s1, const uint16_t* s2, uint16_t max_len)
{
    int idx = 0;
    while (*s1 != 0 || *s2 != 0) {
        if (*s1 == 0) {
            dest[idx++] = *s2++;
        } else if (*s2 == 0) {
            dest[idx++] = *s1++;
        } else if (*s1 < *s2) {
            dest[idx++] = *s1++;
        } else {
            dest[idx++] = *s2++;
        }
        if (idx >= max_len) {
            break;
        }
    }
    return idx;
}

int spn_dcp_ident_resp_parse(void* payload, uint16_t len, struct spn_dcp_ident_resp* resp)
{
    struct spn_dcp_general_block* block_ptr;
    uint16_t block_len;
    uint16_t block_type;
    char* r_payload;
    unsigned int idx;
    uint16_t offset;
    int res = SPN_OK;

    r_payload = (char*)payload;

    for (offset = 0; offset < len;) {
        LWIP_ASSERT("DCP: At least 16bit align", (offset & 1) == 0);

        block_ptr = (struct spn_dcp_general_block*)((char*)payload + offset);
        block_len = lwip_htons(block_ptr->dcp_block_length);
        block_type = BLOCK_TYPE(block_ptr->option, block_ptr->sub_option);
        r_payload = (char*)(block_ptr + 1);

        if (block_len + sizeof(*block_ptr) + offset > len) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_HALT, ("DCP: invalid block_len=%d, len=%d, offset=%d\n", block_len, len, offset));
            return -SPN_EBADMSG;
        }

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_MAC_ADDRESS):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.MAC.Address\n"));
            resp->block_info = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            memcpy(resp->mac_addr, r_payload + 2, 6);
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Mac=%02x:%02x:%02x:%02x:%02x:%02x", resp->mac_addr[0], resp->mac_addr[1], resp->mac_addr[2], resp->mac_addr[3], resp->mac_addr[4], resp->mac_addr[5]));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Parameter\n"));
            LWIP_ASSERT("Invalid block length", block_len == 14);
            resp->block_info = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            resp->ip_addr = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 2));
            resp->ip_mask = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 6));
            resp->ip_gw = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 10));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Parameter=0x%04x, ip=0x%08x, mask=0x%08x, gw=0x%08x\n", resp->block_info, resp->ip_addr, resp->ip_mask, resp->ip_gw));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Full.IPSuite\n"));
            LWIP_ASSERT("invalid block length", block_len == 30);
            resp->block_info = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            resp->ip_addr = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 2));
            resp->ip_mask = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 6));
            resp->ip_gw = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 10));
            resp->ip_dns[0] = lwip_htons(GET_VALUE(r_payload, uint32_t, 14));
            resp->ip_dns[1] = lwip_htons(GET_VALUE(r_payload, uint32_t, 18));
            resp->ip_dns[2] = lwip_htons(GET_VALUE(r_payload, uint32_t, 22));
            resp->ip_dns[3] = lwip_htons(GET_VALUE(r_payload, uint32_t, 26));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Full.IPSuite=0x%04x, ip=0x%08x, mask=0x%08x, gw=0x%08x, dns=%d.%d.%d.%d\n", resp->block_info, resp->ip_addr, resp->ip_mask, resp->ip_gw, resp->ip_dns[0], resp->ip_dns[1], resp->ip_dns[2], resp->ip_dns[3]));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.Vendor\n"));
            resp->vendor_of_name = (char*)malloc(block_len - 1);
            if (!resp->vendor_of_name) {
                res = -SPN_ENOMEM;
                goto free_out;
            }
            strncpy(resp->vendor_of_name, r_payload + 2, block_len - 2);
            resp->vendor_of_name[block_len - 2] = '\0';
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Vendor=%s\n", resp->vendor_of_name));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.NameOfStation\n"));
            resp->station_of_name = (char*)malloc(block_len - 1);
            if (!resp->station_of_name) {
                res = -SPN_ENOMEM;
                goto free_out;
            }
            strncpy(resp->station_of_name, r_payload + 2, block_len - 2);
            resp->station_of_name[block_len - 2] = '\0';
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: NameOfStation=%s\n", resp->station_of_name));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceID\n"));
            LWIP_ASSERT("Invalid block length", block_len == 6);
            resp->vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            resp->device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 4));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceID=0x%04x:0x%04x\n", resp->vendor_id, resp->device_id));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceRole\n"));
            LWIP_ASSERT("Invalid block length", block_len == 4); /* RoleBLock has 1 byte padding */
            resp->device_role = GET_VALUE(r_payload, uint8_t, 2);
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceRole=%d\n", resp->device_role));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceOptions\n"));
            LWIP_ASSERT("Invalid block length", block_len >= 2);
            resp->options = (uint16_t*)malloc(block_len);
            if (!resp->options) {
                res = -SPN_ENOMEM;
                goto free_out;
            }
            for (idx = 0; idx < (unsigned)(block_len / 2 - 1); idx++) {
                resp->options[idx] = lwip_htons(GET_VALUE(r_payload, uint16_t, 2 + idx * 2));
                LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_FRESH, ("DCP: DeviceOptions[%d]=0x%04x\n", idx, resp->options[idx]));
            }
            resp->options[idx] = 0; /* EOF is matter */
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceOptions num=%d\n", idx));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.AliasName\n"));
            resp->alias_of_name = (char*)malloc(block_len - 1);
            if (!resp->alias_of_name) {
                res = -SPN_ENOMEM;
                goto free_out;
            }
            strncpy(resp->alias_of_name, r_payload + 2, block_len - 2);
            resp->alias_of_name[block_len - 2] = '\0';
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: AliasName=%s\n", resp->alias_of_name));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceInstance\n"));
            LWIP_ASSERT("Invalid block length", block_len == 4);
            resp->dev_instance = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceInstance=0x%04x\n", resp->dev_instance));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.OEMDeviceID\n"));
            LWIP_ASSERT("Invalid block length", block_len == 6);
            resp->oem_vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            resp->oem_device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 4));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: OEMDeviceID=0x%04x:0x%04x\n", resp->oem_vendor_id, resp->oem_device_id));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.StandardGateway\n"));
            LWIP_ASSERT("Invalid block length", block_len == 4);
            resp->std_gw_flag = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: StandardGateway=0x%02x\n", resp->std_gw_flag));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.RSIProperties\n"));
            LWIP_ASSERT("Invalid block length", block_len == 4);
            resp->rsi_prop_value = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: RSIProperties=0x%02x\n", resp->rsi_prop_value));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DHCP.DHCP\n"));
            LWIP_ASSERT("Invalid block length", block_len > 4);
            resp->dhcp_option = GET_VALUE(r_payload, uint8_t, 2);
            resp->dhcp_param_len = GET_VALUE(r_payload, uint8_t, 3);
            resp->dhcp_param = (uint8_t*)malloc(resp->dhcp_param_len);
            if (!resp->dhcp_param) {
                res = -SPN_ENOMEM;
                goto free_out;
            }
            memcpy(resp->dhcp_param, r_payload + 4, resp->dhcp_param_len);
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DHCP=%d, len=%d\n", resp->dhcp_option, resp->dhcp_param_len));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DeviceInitiative.DeviceInitiative\n"));
            LWIP_ASSERT("Invalid block length", block_len == 4);
            resp->dev_initiative_flag = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceInitiative=0x%02x\n", resp->dev_initiative_flag));
            break;

        /* TODO: For TSN feature, not supported */
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PARAMETER_UUID):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
        /* NOTE: Invalid block in ident.req */
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_START):
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_STOP):
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_SIGNAL):
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESPONSE):
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_FACTORY_RESET):
        case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESET_TO_FACTORY):
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Reserved block_type=0x%04x\n", block_type));
            break;
        default:
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown block_type=0x%04x\n", block_type));
        }

        offset = spn_dcp_block_walk(payload, offset);
    }

    return res;
free_out:
    if (resp->vendor_of_name) {
        free(resp->vendor_of_name);
    }
    if (resp->station_of_name) {
        free(resp->station_of_name);
    }
    if (resp->options) {
        free(resp->options);
    }
    if (resp->alias_of_name) {
        free(resp->alias_of_name);
    }
    if (resp->dhcp_param) {
        free(resp->dhcp_param);
    }
    return res;
}

int spn_dcp_ident_resp_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_ident_req* reqs, iface_t* iface)
{
    struct pbuf* p;
    char* r_payload;
    int res = SPN_OK;
    int len;
    uint16_t offset = 0;
    const int16_t header_size = sizeof(struct spn_dcp_header) + sizeof(struct pn_pdu) + SIZEOF_ETH_HDR;
    const uint16_t hdr_size = sizeof(struct spn_dcp_general_block) + 2;
    uint16_t block_info = (uint16_t)spn_sys_get_ip_status();
    struct spn_dcp_header* dcp_hdr;
    struct pn_pdu* pn_hdr;

    /* merge two list into one in asc order, list EOF check: value==0 */
    uint16_t merged[32];
    unsigned i, k;
    k = spn_dcp_merge_options(merged, reqs->option_sub_option, spn_dcp_mandatory_reqs, sizeof(merged) / sizeof(merged[0]));

    p = pbuf_alloc(PBUF_RAW, SPN_DCP_DATA_MAX_LENGTH + SIZEOF_ETH_HDR + sizeof(struct pn_pdu), PBUF_RAM);
    if (!p) {
        res = -SPN_ENOMEM;
        goto out;
    }
    pbuf_header(p, -header_size);
    r_payload = (char*)p->payload;

    /* Fill blocks */
    for (i = 0; i < k; i++) {
        uint16_t block_type = merged[i];

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 12, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size,
                spn_sys_get_ip_addr(iface),
                spn_sys_get_ip_mask(iface),
                spn_sys_get_ip_gw(iface));
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 28, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size,
                spn_sys_get_ip_addr(iface),
                spn_sys_get_ip_mask(iface),
                spn_sys_get_ip_gw(iface));
            spn_dcp_pack_dns(r_payload + offset + hdr_size + 12,
                spn_sys_get_dns(iface, 0),
                spn_sys_get_dns(iface, 1),
                spn_sys_get_dns(iface, 2),
                spn_sys_get_dns(iface, 3));
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            len = spn_dcp_pack_vendor_name(r_payload + offset + hdr_size, spn_sys_get_vendor_name());
            spn_dcp_pack_resp_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            len = spn_dcp_pack_station_of_name(r_payload + offset + hdr_size, spn_sys_get_station_name());
            spn_dcp_pack_resp_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_device_id(r_payload + offset + hdr_size, spn_sys_get_vendor_id(), spn_sys_get_device_id());
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_role(r_payload + offset + hdr_size, spn_sys_get_role());
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            len = spn_dcp_pack_options(r_payload + offset + hdr_size, spn_dcp_supported_options);
            spn_dcp_pack_resp_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            len = spn_dcp_pack_alias(r_payload + offset + hdr_size, spn_sys_get_alias_name());
            spn_dcp_pack_resp_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_instance(r_payload + offset + hdr_size, 0); /* TOD: As default, we only support single instance */
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_oem_id(r_payload + offset + hdr_size, spn_sys_get_oem_vendor_id(), spn_sys_get_oem_device_id());
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            spn_dcp_pack_resp_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_device_initiative(r_payload + offset, lwip_htons(SPN_DCP_DEV_INITIATIVE_ENABLE_HELLO));
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            break;
        default:
            res = -SPN_ENOSYS;
            goto free_out;
        }

        /* Align to 16-bit */
        offset = (offset + 1) & ~1;
        if (offset > SPN_DCP_DATA_MAX_LENGTH) {
            res = -SPN_EMSGSIZE;
            goto free_out;
        }
    }

    pbuf_header(p, sizeof(*dcp_hdr));
    dcp_hdr = (struct spn_dcp_header*)p->payload;
    dcp_hdr->service_id = SPN_DCP_SERVICE_ID_IDENTIFY;
    dcp_hdr->service_type = SPN_DCP_SERVICE_TYPE_RESPONSE;
    dcp_hdr->dcp_data_length = lwip_htons(offset);
    dcp_hdr->xid = lwip_ntohl(reqs->xid);

    pbuf_header(p, sizeof(*pn_hdr));
    pn_hdr = (struct pn_pdu*)p->payload;
    pn_hdr->frame_id = PP_HTONS(FRAME_ID_DCP_IDENT_RES);

    ethernet_output(iface, p, (const struct eth_addr*)iface->hwaddr, &hw_hdr->src, ETHTYPE_PROFINET);
free_out:
    pbuf_free(p);
out:
    return res;
}
