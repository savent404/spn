#include <lwip/opt.h>
#include <spn/errno.h>

#include <lwip/debug.h>
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
    void* payload = (void*)((char*)pdu + sizeof(*pdu));
    size_t payload_len = len - sizeof(*pdu);

    LWIP_UNUSED_ARG(iface);

    if (frame_id == FRAME_ID_PTCP_1 || frame_id == FRAME_ID_PTCP_2) {
        /* TODO: PTCP input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found ptcp frame, frame_id=0x%04x\n", frame_id));
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC3)) {
        /* TODO: RTC input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found rtc3 frame, frame_id=0x%04x\n", frame_id));
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC2)) {
        /* TODO: RTC input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found rtc2 frame, frame_id=0x%04x\n", frame_id));
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC1)) {
        /* TODO: RTC input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found rtc1 frame, frame_id=0x%04x\n", frame_id));
    } else if (FRAME_ID_IN_RANGE(frame_id, RTC_UDP)) {
        /* TODO: RTC_UDP input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found rtc_udp frame, frame_id=0x%04x\n", frame_id));
    } else if (frame_id == FRAME_ID_ALARM_HIGH_1 || frame_id == FRAME_ID_ALARM_HIGH_2) {
        /* TODO: ALARM input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found alarm_high frame, frame_id=0x%04x\n", frame_id));
    } else if (frame_id == FRAME_ID_ALARM_LOW_1 || frame_id == FRAME_ID_ALARM_LOW_2) {
        /* TODO: ALARM input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found alarm_low frame, frame_id=0x%04x\n", frame_id));
    } else if (frame_id == FRAME_ID_RTA || frame_id == FRAME_ID_RTA_SECURITY) {
        /* TODO: RTA input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found rta frame, frame_id=0x%04x\n", frame_id));
    } else if (frame_id == FRAME_ID_DCP_HELLO_REQ || frame_id == FRAME_ID_DCP_GET_SET || frame_id == FRAME_ID_DCP_IDENT_REQ || frame_id == FRAME_ID_DCP_IDENT_RES) {
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found dcp frame, frame_id=0x%04x\n", frame_id));
        return spn_dcp_input(payload, payload_len, frame_id, hw_hdr, iface);
    } else if (frame_id == FRAME_ID_PTCP_ANNOUCE || frame_id == FRAME_ID_PTCP_FOLLOW_UP || frame_id == FRAME_ID_PTCP_DELAY_REQ || frame_id == FRAME_ID_PTCP_DELAY_RES_1 || frame_id == FRAME_ID_PTCP_DELAY_RES_2 || frame_id == FRAME_ID_PTCP_DELAY_RES_3) {
        /* TODO: PTCP input */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: found ptcp frame, frame_id=0x%04x\n", frame_id));
    } else {
        /* TODO: default error handler */
        LWIP_DEBUGF(SPN_PDU_DEBUG | LWIP_DBG_TRACE, ("PDU: unknown frame, frame_id=0x%04x\n", frame_id));
    }
    return SPN_OK;
}

extern err_t
ethernet_output(struct netif* netif, struct pbuf* p,
    const struct eth_addr* src, const struct eth_addr* dst,
    u16_t eth_type);

int spn_pdu_rtc_output(struct pbuf* p, struct eth_addr* src, struct eth_addr* dest, uint16_t frame_id, iface_t* iface)
{
    struct pn_pdu* pdu;

    if (pbuf_add_header(p, SPN_PDU_HDR_SIZE)) {
        return -SPN_ENOMEM;
    }

    pdu = (struct pn_pdu*)p->payload;
    pdu->frame_id = PP_HTONS(frame_id);

    return ethernet_output(iface, p, src, dest, PP_HTONS(ETHTYPE_PROFINET));
}
