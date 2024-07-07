#include <lwip/opt.h>
#include <spn/errno.h>

#include <lwip/prot/ethernet.h>
#include <spn/dcp.h>
#include <spn/pdu.h>

#define _FRAME_ID_IN_RANGE(frame_id, begin, end) \
    ((frame_id) <= (end))
#define FRAME_ID_IN_RANGE(frame_id, RANGE) \
    _FRAME_ID_IN_RANGE(frame_id, FRAME_ID_##RANGE##_BEGIN, FRAME_ID_##RANGE##_END)

int spn_pdu_input(void* frame, size_t len, iface_t* iface)
{
    struct pn_pdu* pdu = (struct pn_pdu*)frame;
    uint16_t frame_id = ntohs(pdu->frame_id);
    void* payload = (void*)((char*)pdu + 2);
    size_t payload_len = len - 2;

    LWIP_UNUSED_ARG(len);
    LWIP_UNUSED_ARG(iface);

    if (FRAME_ID_IN_RANGE(frame_id, TIME_SYNC)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, RTC3)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, RTC2)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, RTC1)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, ACYCLIC_HIGH_TX)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, RESERVED)) {
        return -SPN_EBADMSG;
    } else if (FRAME_ID_IN_RANGE(frame_id, ACYCLIC_LOW_TX)) {

    } else if (FRAME_ID_IN_RANGE(frame_id, DCP)) {
        return spn_dcp_input(payload, payload_len, frame_id, iface);
    } else {
        return -SPN_EBADMSG;
    }
    return SPN_OK;
}
