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

#ifndef SPN_IFACE_REALTIME_TIMEOUT
#define SPN_IFACE_REALTIME_TIMEOUT (10) // 10us
#endif

typedef struct spn_iface {
    int port;
    uint8_t ip[4];
    uint8_t mac[6];
} spn_iface_t;

bool spn_iface_input_poll(int port, void* frame, size_t len, uint32_t timeout);
bool spn_iface_output_poll(int port, const void* frame, size_t len, uint32_t timeout);
bool spn_iface_output(int port, const void* frame, size_t len);
bool spn_iface_set_ip_address(int port, const char* ip_address);
bool spn_iface_get_ip_address(int port, uint8_t* address);
bool spn_iface_get_mac_address(int port, uint8_t* mac_address);

#ifdef __cplusplus
}
#endif
