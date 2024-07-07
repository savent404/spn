#include <lwip/opt.h>
#include <spn/errno.h>

#include <lwip/prot/ethernet.h>
#include <spn/dcp.h>
#include <spn/pdu.h>

#define _FRAME_ID_IN_RANGE(frame_id, begin, end) \
    ((frame_id) <= (end) && frame_id >= (begin))
#define FRAME_ID_IN_RANGE(frame_id, RANGE) \
    _FRAME_ID_IN_RANGE(frame_id, FRAME_ID_##RANGE##_BEGIN, FRAME_ID_##RANGE##_END)

int spn_pdu_input(void* frame, size_t len, struct eth_hdr* hw_hdr, iface_t* iface)
{
    struct pn_pdu* pdu = (struct pn_pdu*)frame;
    uint16_t frame_id = ntohs(pdu->frame_id);
    void* payload = (void*)((char*)pdu + 2);
    size_t payload_len = len - 2;

    LWIP_UNUSED_ARG(len);
    LWIP_UNUSED_ARG(iface);

    if (frame_id == FRAME_ID_PTCP_1 || frame_id == FRAME_ID_PTCP_2) {
        /* TODO: PTCP input */
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC3)) {
        /* TODO: RTC input */
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC2)) {
        /* TODO: RTC input */
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC1)) {
        /* TODO: RTC input */
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC_UDP)) {
        /* TODO: RTC_UDP input */
    } else if (frame_id == FRAME_ID_ALARM_HIGH_1 || frame_id == FRAME_ID_ALARM_HIGH_2) {
        /* TODO: ALARM input */
    } else if (frame_id == FRAME_ID_ALARM_LOW_1 || frame_id == FRAME_ID_ALARM_LOW_2) {
        /* TODO: ALARM input */
    } else if (frame_id == FRAME_ID_RTA || frame_id == FRAME_ID_RTA_SECURITY) {
        /* TODO: RTA input */
    } else if (frame_id == FRAME_ID_DCP_HELLO_REQ || frame_id == FRAME_ID_DCP_GET_SET || frame_id == FRAME_ID_DCP_IDENT_REQ || frame_id == FRAME_ID_DCP_IDENT_RES) {
        return spn_dcp_input(payload, payload_len, frame_id, hw_hdr, iface);
    } else if (frame_id == FRAME_ID_PTCP_ANNOUCE || frame_id == FRAME_ID_PTCP_FOLLOW_UP || frame_id == FRAME_ID_PTCP_DELAY_REQ || frame_id == FRAME_ID_PTCP_DELAY_RES_1 || frame_id == FRAME_ID_PTCP_DELAY_RES_2 || frame_id == FRAME_ID_PTCP_DELAY_RES_3) {
        /* TODO: PTCP input */
    } else {
        /* TODO: default error handler */
    }
    return SPN_OK;
}
