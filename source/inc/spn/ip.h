#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPN_IP_VER_IHL 0x45 // 4-bit IPv4 version, 4-bit header length (5 * 4 = 20 bytes)

#define SPN_IP_PROTO_ICMP 1
#define SPN_IP_PROTO_TCP 6
#define SPN_IP_PROTO_UDP 17

#pragma pack(push, 1)
typedef struct ip_hdr {
    uint8_t ver_ihl; // Version (4 bits) + Internet header length (4 bits)
    uint8_t tos; // Type of service
    uint16_t tlen; // Total length
    uint16_t identification; // Identification
    uint16_t flags_fo; // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t ttl; // Time to live
    uint8_t proto; // Protocol
    uint16_t crc; // Header checksum
    uint32_t saddr; // Source address
    uint32_t daddr; // Destination address
    uintptr_t payload[0];
} ip_hdr_t;
#pragma pack(pop)

int spn_ip_input(void* ip_frame, size_t len);
int spn_ip_output(void* ip_frame, size_t len);

#ifdef __cplusplus
}
#endif