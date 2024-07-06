/**
 * @file spn_if.h
 * @author savent (savent_gate@outlook.com)
 * @brief iface interface for spn
 * @date 2024-07-05
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPN_IFACE_MAX_NUM
#define SPN_IFACE_MAX_NUM (2)
#endif

#ifndef SPN_IFACE_REALTIME_TIMEOUT
#define SPN_IFACE_REALTIME_TIMEOUT (10) // 10us
#endif

#ifndef SPN_IFACE_FRAME_MAX_LEN
#define SPN_IFACE_FRAME_MAX_LEN (1518)
#endif

struct spn_instance;
typedef struct spn_instance spn_instance_t;

typedef struct spn_iface {
    int port;
    uint8_t ip[4];
    uint8_t mac[6];
    spn_instance_t* inst;
} spn_iface_t;

#define SPN_IFACE_IP(ip_address) (*(uint32_t*)(ip_address))
#define SPN_IFACE_MAC_H(mac_address) (*(uint16_t*)(mac_address))
#define SPN_IFACE_MEM_L(mac_address) (*(uint32_t*)(mac_address + 2))

bool spn_iface_input_poll(int port, void* frame, size_t len, uint32_t timeout);
bool spn_iface_output_poll(int port, const void* frame, size_t len, uint32_t timeout);
bool spn_iface_output(int port, const void* frame, size_t len);
bool spn_iface_set_ip_address(int port, const char* ip_address);
bool spn_iface_get_ip_address(int port, uint8_t* address);
bool spn_iface_get_mac_address(int port, uint8_t* mac_address);

#ifdef __cplusplus
}
#endif
