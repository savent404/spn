#include <lwip/opt.h>
#include <lwip/sys.h>
#include <netif/ethernet.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <string.h>

int spn_dcp_ident_req_parse(void* payload, uint16_t len, struct spn_dcp_ident_req* reqs, iface_t* iface)
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
        LWIP_ASSERT("DCP: At least 16bit align", (offset & 1) == 0);
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
            if (spn_dcp_filter_ip(iface, ip_addr, ip_mask, ip_gw)) {
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
            if (spn_dcp_filter_ip(iface, ip_addr, ip_mask, ip_gw)) {
                goto filter_miss_match;
            }
            if (spn_dcp_filter_dns(iface, ip_dns[0], ip_dns[1], ip_dns[2], ip_dns[3])) {
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
            if (spn_dcp_filter_options((const uint16_t*)((uintptr_t)r_payload & ~1), block_len / 2)) {
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

int spn_dcp_ident_req_assemble(const uint16_t* options, struct spn_dcp_ident_resp* resp, iface_t* iface)
{
    struct pbuf* p;
    char* r_payload;
    int res = SPN_OK;
    int len;
    uint16_t offset = 0;
    uint16_t frame_len;
    const int16_t header_size = sizeof(struct spn_dcp_header) + sizeof(struct pn_pdu) + SIZEOF_ETH_HDR;
    const uint16_t hdr_size = sizeof(struct spn_dcp_general_block);
    const struct eth_addr broadcast_addr = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
    struct spn_dcp_header* dcp_hdr;
    struct pn_pdu* pn_hdr;
    unsigned i, k;

    if (!options || !resp) {
        return -SPN_EINVAL;
    }

    /* Count options number */
    for (k = 0; options[k] && k < 8; k++) {
    }
    if (k == 0 || k >= 8) {
        res = -SPN_EINVAL;
        goto out;
    }

    p = pbuf_alloc(PBUF_RAW, SPN_DCP_DATA_MAX_LENGTH + SIZEOF_ETH_HDR + sizeof(struct pn_pdu), PBUF_RAM);
    if (!p) {
        res = -SPN_ENOMEM;
        goto out;
    }
    pbuf_header(p, -header_size);
    r_payload = (char*)p->payload;

    /* Fill blocks */
    for (i = 0; i < k; i++) {
        uint16_t block_type = options[i];

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 12);
            spn_dcp_pack_ip(r_payload + offset + hdr_size,
                lwip_htons(resp->ip_addr),
                lwip_htons(resp->ip_mask),
                lwip_htons(resp->ip_gw));
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 28);
            spn_dcp_pack_ip(r_payload + offset + hdr_size,
                lwip_htons(resp->ip_addr),
                lwip_htons(resp->ip_mask),
                lwip_htons(resp->ip_gw));
            spn_dcp_pack_dns(r_payload + offset + hdr_size + 12,
                lwip_htons(resp->ip_dns[0]),
                lwip_htons(resp->ip_dns[1]),
                lwip_htons(resp->ip_dns[2]),
                lwip_htons(resp->ip_dns[3]));
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            LWIP_ASSERT("name is NULL", resp->vendor_of_name != NULL);
            len = spn_dcp_pack_vendor_name(r_payload + offset + hdr_size, resp->vendor_of_name);
            spn_dcp_pack_req_block(r_payload + offset, block_type, len);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            LWIP_ASSERT("name is NULL", resp->station_of_name != NULL);
            len = spn_dcp_pack_station_of_name(r_payload + offset + hdr_size, resp->station_of_name);
            spn_dcp_pack_req_block(r_payload + offset, block_type, len);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 4);
            spn_dcp_pack_device_id(r_payload + offset + hdr_size, lwip_htons(resp->vendor_id), lwip_htons(resp->device_id));
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 2);
            spn_dcp_pack_role(r_payload + offset + hdr_size, (enum spn_role)resp->device_role);
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            LWIP_ASSERT("name is NULL", resp->alias_of_name != NULL);
            len = spn_dcp_pack_alias(r_payload + offset + hdr_size, resp->alias_of_name);
            spn_dcp_pack_req_block(r_payload + offset, block_type, len);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 2);
            spn_dcp_pack_instance(r_payload + offset + hdr_size, 0); /* TOD: As default, we only support single instance */
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 4);
            spn_dcp_pack_oem_id(r_payload + offset + hdr_size, lwip_htons(resp->vendor_id), lwip_htons(resp->device_id));
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 2);
            spn_dcp_pack_device_initiative(r_payload + offset, lwip_htons(SPN_DCP_DEV_INITIATIVE_ENABLE_HELLO));
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            spn_dcp_pack_req_block(r_payload + offset, block_type, 0);
            offset += hdr_size;
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
    dcp_hdr->service_type = SPN_DCP_SERVICE_TYPE_REQUEST;
    dcp_hdr->dcp_data_length = lwip_htons(offset);
    dcp_hdr->response_delay_factor = lwip_htons(1);
    dcp_hdr->xid = lwip_htonl(resp->xid);

    pbuf_header(p, sizeof(*pn_hdr));
    pn_hdr = (struct pn_pdu*)p->payload;
    pn_hdr->frame_id = PP_HTONS(FRAME_ID_DCP_IDENT_REQ);

    /* Adjust output packet size */
    frame_len = offset + sizeof(*dcp_hdr) + sizeof(*pn_hdr);
    if (frame_len < SPN_DCP_DATA_MIN_LENGTH) {
        /* Set padding to zero */
        memset(r_payload + offset, 0, SPN_DCP_DATA_MIN_LENGTH - frame_len);
        frame_len = SPN_DCP_DATA_MIN_LENGTH;
    }
    p->len = p->tot_len = frame_len;
    LOCK_TCPIP_CORE();
    ethernet_output(iface, p, (const struct eth_addr*)iface->hwaddr, &broadcast_addr, ETHTYPE_PROFINET);
    UNLOCK_TCPIP_CORE();
free_out:
    pbuf_free(p);
out:
    return res;
}
