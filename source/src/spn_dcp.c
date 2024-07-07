#include <lwip/arch.h>
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

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, iface_t* iface)
{
    struct spn_dcp_header* dcp_hdr = (struct spn_dcp_header*)frame;
    uint16_t dcp_data_len = PP_HTONS(dcp_hdr->dcp_data_length);
    uint8_t dcp_service_id = dcp_hdr->service_id;

    LWIP_UNUSED_ARG(len);
    LWIP_UNUSED_ARG(frame_id);
    LWIP_UNUSED_ARG(iface);

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
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
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
    return -SPN_EINVAL;

err_invalid_service_id:
    /* TODO: send error response */
    return SPN_OK;
}
