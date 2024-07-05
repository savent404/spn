/**
 * \file spn_os.h
 * \author savent (savent_gate@outlook.com)
 * \brief spn os interface
 * \date 2024-07-05
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t spn_os_mailbox_t;
typedef uintptr_t spn_os_timer_t;

typedef enum spn_os_priority {
    SPN_OS_PRIORITY_IDLE = 0,
    SPN_OS_PRIORITY_LOW,
    SPN_OS_PRIORITY_NORMAL,
    SPN_OS_PRIORITY_HIGH,
    SPN_OS_PRIORITY_REALTIME,
} spn_os_priority_t;

typedef enum spn_os_timer_type {
    SPN_OS_TIMER_PERIODIC = 0,
    SPN_OS_TIMER_ONESHOT,
} spn_os_timer_type_t;

typedef struct spn_os_time_t {
    uint32_t sec;
    uint32_t nano_sec;
} spn_os_time_t;

spn_os_mailbox_t spn_mailbox_create(const char* name, size_t size, spn_os_priority_t prio);
bool spn_mailbox_tx(spn_os_mailbox_t mailbox, uintptr_t msg);
bool spn_mailbox_rx(spn_os_mailbox_t mailbox, uintptr_t* msg, uint32_t timeout);
void spn_mailbox_destroy(spn_os_mailbox_t mailbox);

spn_os_timer_t spn_timer_create(const char* name,
    spn_os_priority_t prio,
    uint32_t period,
    spn_os_timer_type_t type,
    void (*callback)(uintptr_t),
    uintptr_t arg);
void spn_timer_start(spn_os_timer_t timer);
void spn_timer_cancel(spn_os_timer_t timer);
void spn_timer_destroy(spn_os_timer_t timer);

void spn_msleep(uint32_t time);
void spn_yield();
void spn_get_time(spn_os_time_t* time);

#ifdef __cplusplus
}
#endif
