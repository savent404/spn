#include <netif/ethernet.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/spn_sys.h>
#include <string.h>

int spn_dcp_set_resp_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_block_req* reqs, iface_t* iface)
{
    struct pbuf* p;
    char* r_payload;
    int res = SPN_OK;
    uint16_t offset = 0;
    const int16_t header_size = sizeof(struct spn_dcp_header) + sizeof(struct pn_pdu) + SIZEOF_ETH_HDR;
    const uint16_t hdr_size = sizeof(struct spn_dcp_general_block);
    uint16_t frame_len;
    struct spn_dcp_header* dcp_hdr;
    struct pn_pdu* pn_hdr;

    /* merge two list into one in asc order, list EOF check: value==0 */

    p = pbuf_alloc(PBUF_RAW, SPN_DCP_DATA_MAX_LENGTH + SIZEOF_ETH_HDR + sizeof(struct pn_pdu), PBUF_RAM);
    if (!p) {
        res = -SPN_ENOMEM;
        goto out;
    }
    pbuf_header(p, -header_size);
    r_payload = (char*)p->payload;

    /* Reserve response option hdr */
    spn_dcp_pack_req_block(r_payload, BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESPONSE), 3);
    offset += hdr_size;
    *(uint16_t*)(r_payload + offset) = lwip_htons(reqs->option_sub_option[0]);
    *(uint8_t*)(r_payload + offset + 2) = 0;
    offset += 3;
    offset = (offset + 1) & ~1;

    pbuf_header(p, sizeof(*dcp_hdr));
    dcp_hdr = (struct spn_dcp_header*)p->payload;
    dcp_hdr->service_id = SPN_DCP_SERVICE_ID_SET;
    dcp_hdr->service_type = SPN_DCP_SERVICE_TYPE_RESPONSE;
    dcp_hdr->dcp_data_length = lwip_htons(offset);
    dcp_hdr->response_delay_factor = lwip_htons(0);
    dcp_hdr->xid = lwip_ntohl(reqs->xid);

    pbuf_header(p, sizeof(*pn_hdr));
    pn_hdr = (struct pn_pdu*)p->payload;
    pn_hdr->frame_id = PP_HTONS(FRAME_ID_DCP_GET_SET);

    /* Adjust output packet size */
    frame_len = offset + sizeof(*dcp_hdr) + sizeof(*pn_hdr);
    if (frame_len < SPN_DCP_DATA_MIN_LENGTH) {
        /* Set padding to zero */
        memset(r_payload + frame_len, 0, SPN_DCP_DATA_MIN_LENGTH - frame_len);
        frame_len = SPN_DCP_DATA_MIN_LENGTH;
    }
    p->len = p->tot_len = frame_len;
    ethernet_output(iface, p, (const struct eth_addr*)iface->hwaddr, &hw_hdr->src, ETHTYPE_PROFINET);
    pbuf_free(p);
out:
    return res;
}
