#include <lwip/opt.h>

#include <lwip/arch.h>
#include <lwip/debug.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>

uint16_t spn_dcp_resp_delay(uint16_t rand, uint16_t resp_delay_factor)
{
    uint32_t t = 10 * (rand % resp_delay_factor);
    if (t > SPN_DCP_RESPONSE_DELAY_MAX)
        t = SPN_DCP_RESPONSE_DELAY_MAX;
    return (uint16_t)(t & 0xFFFF);
}

uint16_t spn_dcp_resp_delay_timeout(uint16_t rand, uint16_t resp_delay_factor)
{
    uint32_t t;
    if (resp_delay_factor == SPN_DCP_RESPONSE_DELAY_FACTOR_DEFAULT) {
        return SPN_DCP_RESPONSE_DELAY_DEFAULT;
    }
    t = 1000 + 10 * (rand % resp_delay_factor);
    t = ((t + 999) / 1000) * 1000;
    if (t > SPN_DCP_RESPONSE_DELAY_MAX) {
        t = SPN_DCP_RESPONSE_DELAY_MAX;
    }
    return (uint16_t)(t & 0xFFFF);
}

bool spn_dcp_support_multicast(uint8_t service_id, uint8_t service_type)
{
    bool is_req = service_type == SPN_DCP_SERVICE_TYPE_REQUEST;
    return is_req && (service_id == SPN_DCP_SERVICE_ID_IDENTIFY || service_id == SPN_DCP_SERVICE_ID_HELLO);
}

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface)
{
    struct spn_dcp_header* dcp_hdr = (struct spn_dcp_header*)frame;
    struct spn_dcp_general_type* dcp_type = (struct spn_dcp_general_type*)((uint8_t*)dcp_hdr + sizeof(struct spn_dcp_header));
    uint16_t dcp_data_len = PP_HTONS(dcp_hdr->dcp_data_length);
    uint32_t dcp_xid = PP_HTONL(dcp_hdr->xid);
    uint8_t dcp_service_id = dcp_hdr->service_id;
    uint8_t dcp_service_type = dcp_hdr->service_type;

    LWIP_UNUSED_ARG(iface);
    LWIP_UNUSED_ARG(hw_hdr);

    LWIP_DEBUGF(0x80 | LWIP_DBG_TRACE, ("DCP: frame_id=0x%04x, service_id=%d, service_type=%d, xid=0x%08x, dcp_data_len=%d\n", frame_id, dcp_service_id, dcp_service_type, dcp_xid, dcp_data_len));

    /* General check go firstly */
    if (dcp_data_len + sizeof(struct spn_dcp_header) > len || dcp_data_len >= SPN_DCP_DATA_MAX_LENGTH) {
        return -SPN_EBADMSG;
    }

    switch (frame_id) {
    case FRAME_ID_DCP_HELLO_REQ:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_HELLO) {
            goto err_invalid_service_id;
        }
        break;
    case FRAME_ID_DCP_GET_SET:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_GET && dcp_service_id != SPN_DCP_SERVICE_ID_SET) {
            goto err_invalid_service_id;
        }
        break;
    case FRAME_ID_DCP_IDENT_REQ:
        /* Available type:
         * IdentifyAll-Req(NameOfStationBlock^AliasNameBlock^IdentifyReqBlock)
         * IdentifyFilter-Req(AllSelectorBlock) */
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
        }

        if (dcp_type->option == SPN_DCP_OPTION_ALL_SELECTOR && dcp_type->sub_option == SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR) {
            LWIP_ASSERT("dcp_data_len must be 4", dcp_data_len == 4);
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IdentifyFilter-Req\n"));
            /* TODO: Response Identify.Res */
        } else {
            /* TODO: Find other available PDU */
            /* TODO: Handle general error frame */
        }
        break;
    case FRAME_ID_DCP_IDENT_RES:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
        }
        break;
    default:
        /* TODO: drop unknow frame_id */
        break;
    }
    return SPN_OK;

err_invalid_service_id:
    /* TODO: send error response */
    return SPN_OK;
}
