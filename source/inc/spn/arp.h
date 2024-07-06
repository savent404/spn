#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef SPN_ARP_MAX_NUM
#define SPN_ARP_MAX_NUM (256)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int spn_arp_ping(uint32_t ip);
int spn_query_mac(uint32_t ip, uint8_t* mac);
int spn_query_ip(uint8_t* mac, uint32_t* ip);

int spn_arp_input(void* arp_frame, size_t len);
int spn_arp_output(void* arp_frame, size_t len);

#ifdef __cplusplus
}
#endif
