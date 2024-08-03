#pragma once

#include <lwip/prot/ethernet.h>
#include <stddef.h>
#include <stdint.h>

#include <spn/iface.h>

#define SPN_PDU_HDR_SIZE (2)

#define FRAME_ID_PTCP_1 0x0020
#define FRAME_ID_PTCP_2 0x0080
#define FRAME_ID_RTC3_BEGIN 0x0100
#define FRAME_ID_RTC3_END 0x0FFF
#define FRAME_ID_RTC2_BEGIN 0x1000
#define FRAME_ID_RTC2_END 0x3FFF
#define FRAME_ID_RTC1_BEGIN 0x8000
#define FRAME_ID_RTC1_END 0xBFFF
#define FRAME_ID_RTC_UDP_BEGIN 0xC000
#define FRAME_ID_RTC_UDP_END 0xFBFF
#define FRAME_ID_ALARM_HIGH_1 0xFC01
#define FRAME_ID_ALARM_HIGH_2 0xFC41
#define FRAME_ID_ALARM_LOW_1 0xFE01
#define FRAME_ID_RTA 0xFE02
#define FRAME_ID_ALARM_LOW_2 0xFE41
#define FRAME_ID_RTA_SECURITY 0xFE42
#define FRAME_ID_DCP_HELLO_REQ 0xFEFC
#define FRAME_ID_DCP_GET_SET 0xFEFD
#define FRAME_ID_DCP_IDENT_REQ 0xFEFE
#define FRAME_ID_DCP_IDENT_RES 0xFEFF
#define FRAME_ID_PTCP_ANNOUCE 0xFF00
#define FRAME_ID_PTCP_FOLLOW_UP 0xFF20
#define FRAME_ID_PTCP_DELAY_REQ 0xFF40
#define FRAME_ID_PTCP_DELAY_RES_1 0xFF41 /* with follow up */
#define FRAME_ID_PTCP_DELAY_RES_2 0xFF42 /* with follow up */
#define FRAME_ID_PTCP_DELAY_RES_3 0xFF43 /* without follow up */

#define SPN_RTC_MINIMAL_FRAME_SIZE 56 /* 60 = 56 + 14(eth_hdr) */
#define SPN_RTC_MAXIMAL_FRAME_SIZE 1440

#pragma pack(push, 1)
struct pn_pdu {
  uint16_t frame_id;
};
#pragma pack(pop)
