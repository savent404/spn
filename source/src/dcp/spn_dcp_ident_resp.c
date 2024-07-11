#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/spn_sys.h>

#include <lwip/opt.h>
#include <netif/ethernet.h>

extern const uint16_t* spn_dcp_mandatory_reqs;

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
            spn_dcp_pack_block(r_payload + offset, block_type, 12, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size, iface);
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            spn_dcp_pack_block(r_payload + offset, block_type, 28, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size, iface);
            spn_dcp_pack_dns(r_payload + offset + hdr_size + 12, iface);
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            len = spn_dcp_pack_vendor_name(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            len = spn_dcp_pack_station_of_name(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            spn_dcp_pack_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_device_id(r_payload + offset + hdr_size);
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            spn_dcp_pack_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_role(r_payload + offset + hdr_size);
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            len = spn_dcp_pack_options(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            len = spn_dcp_pack_alias(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            spn_dcp_pack_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_instance(r_payload + offset + hdr_size);
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            spn_dcp_pack_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_oem_id(r_payload + offset + hdr_size);
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
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
    dcp_hdr->xid = lwip_htonl(0x12345678); /* TODO: Resp the same xid as ident.req */

    pbuf_header(p, sizeof(*pn_hdr));
    pn_hdr = (struct pn_pdu*)p->payload;
    pn_hdr->frame_id = PP_HTONS(FRAME_ID_DCP_IDENT_RES);

    ethernet_output(iface, p, (const struct eth_addr*)iface->hwaddr, &hw_hdr->src, ETHTYPE_PROFINET);
free_out:
    pbuf_free(p);
out:
    return res;
}
