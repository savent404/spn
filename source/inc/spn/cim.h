#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int cim_default_t;

typedef cim_default_t cim_interface_t;
typedef cim_default_t cim_port_t;

typedef struct cim_ctx {
  int reserved;
  void* db_ptr;
} cim_ctx_t;

typedef struct cim_db_object {
  uint8_t option;
  uint8_t sub_option;
  void* data;
  size_t len;
} cim_db_obj_t;

typedef enum cim_id {
  CIM_ID_NAME_OF_STATION,
} cim_id_t;

int cim_init(cim_ctx_t* ctx);

int cim_read_interface_data(cim_ctx_t* ctx, cim_interface_t interface, cim_port_t port, void* data, size_t len);
int cim_write_interface_data(cim_ctx_t* ctx, cim_interface_t interface, cim_port_t port, void* data, size_t len);