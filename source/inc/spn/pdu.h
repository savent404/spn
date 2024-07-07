#pragma once

#include <stddef.h>
#include <stdint.h>

#include <spn/iface.h>

#define FRAME_ID_TIME_SYNC_BEGIN 0x0000
#define FRAME_ID_TIME_SYNC_END 0x00FF
#define FRAME_ID_RTC3_BEGIN 0x0100
#define FRAME_ID_RTC3_END 0x7FFF
#define FRAME_ID_RTC2_BEGIN 0x8000
#define FRAME_ID_RTC2_END 0xBFFF
#define FRAME_ID_RTC1_BEGIN 0xC000
#define FRAME_ID_RTC1_END 0xFBFF
#define FRAME_ID_ACYCLIC_HIGH_TX_BEGIN 0xFC00
#define FRAME_ID_ACYCLIC_HIGH_TX_END 0xFCFF
#define FRAME_ID_RESERVED_BEGIN 0xFD00
#define FRAME_ID_RESERVED_END 0xFEFF
#define FRAME_ID_ACYCLIC_LOW_TX_BEGIN 0xFE00
#define FRAME_ID_ACYCLIC_LOW_TX_END 0xFEFC
#define FRAME_ID_DCP_BEGIN 0xFEFD
#define FRAME_ID_DCP_END 0xFEFF

#pragma pack(push, 1)
struct pn_pdu {
    uint16_t frame_id;
    union {
        uint32_t _reserved;
    } payload;
};
#pragma pack(pop)

int spn_pdu_input(void* frame, size_t len, iface_t* iface);
