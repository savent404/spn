#include <spn/dcp.h>
#include <spn/errno.h>

struct spn_dcp_ctx dcp_ctx;

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface)
{
    struct spn_dcp_header* dcp_hdr = (struct spn_dcp_header*)frame;
    struct spn_dcp_general_block* dcp_blocks = (struct spn_dcp_general_block*)((uint8_t*)dcp_hdr + sizeof(struct spn_dcp_header));
    uint16_t dcp_data_len = lwip_htons(dcp_hdr->dcp_data_length);
    uint32_t dcp_xid = lwip_htonl(dcp_hdr->xid);
    uint8_t dcp_service_id = dcp_hdr->service_id;
    uint8_t dcp_service_type = dcp_hdr->service_type;
    int res;
    uint32_t frame_id_service_id = (frame_id << 8) | dcp_service_id;

    LWIP_UNUSED_ARG(iface);
    LWIP_UNUSED_ARG(hw_hdr);

    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: frame_id=0x%04x, service_id=%d, service_type=%d, xid=0x%08x, dcp_data_len=%d\n", frame_id, dcp_service_id, dcp_service_type, dcp_xid, dcp_data_len));

    /* General check go firstly */
    if (dcp_data_len + sizeof(struct spn_dcp_header) > len || dcp_data_len >= SPN_DCP_DATA_MAX_LENGTH) {
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: invalid dcp_data_len=%d, frame_len=%ld\n", dcp_data_len, len));
        return -SPN_EBADMSG;
    }

    /* check rules related with frame_id and service_id&service_id */
    switch (frame_id_service_id) {
    case FRAME_ID_DCP_HELLO_REQ << 8 | SPN_DCP_SERVICE_ID_HELLO:
        /* TODO: Use spn_dcp_hello_req_parse */
        /* spn_dcp_block_parse(dcp_blocks, dcp_data_len, 0, 0, &blocks); */
        break;
    case FRAME_ID_DCP_GET_SET << 8 | SPN_DCP_SERVICE_ID_GET:
    case FRAME_ID_DCP_GET_SET << 8 | SPN_DCP_SERVICE_ID_SET:
        /* TODO: Use spn_dcp_get_set_parse */
        /* spn_dcp_block_parse(dcp_blocks, dcp_data_len, 0, 0, &blocks); */
        break;
    case FRAME_ID_DCP_IDENT_REQ << 8 | SPN_DCP_SERVICE_ID_IDENTIFY: {
        /**
         * Steps:
         *  1. Parse the DCP blocks
         *  2. Assemble the response
         * @todo need schedule the response by response_delay_factory
         */
        struct spn_dcp_ident_req req = { 0 };
        res = spn_dcp_ident_req_parse(dcp_blocks, dcp_data_len, &req);
        if (res == SPN_OK) {
            req.xid = dcp_xid;
            res = spn_dcp_ident_resp_assemble(hw_hdr, &req, iface);
        } else {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: spn_dcp_ident_req_parse failed, res=%d\n", res));
            break;
        }
        break;
    }
    case FRAME_ID_DCP_IDENT_RES << 8 | SPN_DCP_SERVICE_ID_IDENTIFY: {
        /**
         * Steps:
         * 1. Check if the response is for me
         * 2. Parse the DCP blocks
         * 3. Register the device
         */
        unsigned session_idx = 0;
        for (session_idx = 0; session_idx < sizeof(dcp_ctx.dev_session) / sizeof(dcp_ctx.dev_session[0]); session_idx++) {
            if (dcp_ctx.dev_session[session_idx].resp.xid == dcp_xid) {
                if (dcp_ctx.dev_session[session_idx].state != dcp_dev_state_ident) {
                    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: xid=%08x is already in used\n", dcp_xid));
                    goto ret;
                }
                goto found_xid;
            }
        }
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: no session found for xid=0x%08x\n", dcp_xid));
        break;

    found_xid:
        res = spn_dcp_ident_resp_parse(dcp_blocks, dcp_data_len, &dcp_ctx.dev_session[session_idx].resp);

        if (res == SPN_OK) {
            dcp_ctx.dev_session[session_idx].state = dcp_dev_state_active;
        } else {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: spn_dcp_ident_resp_parse failed, res=%d\n", res));
            break;
        }
    } break;
    default:
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown dcp frame, frame_id=0x%04x, service_id=%d\n", frame_id, dcp_service_id));
        break;
    }

ret:
    return SPN_OK;
}
