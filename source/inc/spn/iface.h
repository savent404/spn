#pragma once

#include <lwip/netif.h>
#include <stdint.h>

typedef struct spn_iface {
  struct netif netif;
} spn_iface_t;

typedef enum frame_type {
  FRAME_TYPE_RTC,
  FRAME_TYPE_RTA,
  FRAME_TYPE_DCP,
  FRAME_TYPE_NUM,
} spn_ftype_t;

typedef void* spn_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

spn_frame_t spn_alloc_frame(spn_iface_t* iface, spn_ftype_t type);
void spn_free_frame(spn_iface_t* iface, spn_frame_t frame);

int spn_send_frame(spn_iface_t* iface, spn_frame_t frame, uint8_t *mac);

void* spn_frame_data(spn_frame_t frame);
uint16_t spn_frame_size(spn_frame_t frame);
void spn_frame_set_size(spn_frame_t frame, uint16_t size);
spn_ftype_t spn_frame_type(spn_frame_t frame);

#ifdef __cplusplus
}
#endif
