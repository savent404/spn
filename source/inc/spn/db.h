#pragma once

#include <spn/config.h>
#include <spn/db_ids.h>
#include <spn/sys.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inherit diagram
 *
 * db_ctx
 * |
 * +-- db_interface [internal]
 * |   |
 * |   +-- id
 * |   +-- db_port [X1]
 * |       |
 * |       +-- id
 * |       +-- db_object_arr
 * |           |
 * |           +-- db_object
 * |           |   |
 * |           |   +-- val
 * |           |   +-- addr
 * |           |   +-- attr
 * |           |   +-- view
 * |           +-- db_object
 * |           +-- db_object
 * |           +-- db_object
 * |           +-- ...
 * +-- db_interface [external]
 *     |
 *     +-- ...
 */

typedef void* db_view_t; /* view is a dynamic object that can be used to notify listener that object has been changed */

typedef struct {
  uint16_t iface;
  uint16_t port;
  enum db_id obj;
} db_addr_t;

typedef struct {
  uint16_t len : 10;
  uint16_t is_dyn : 1;
  uint16_t is_arr : 1;
  uint16_t is_valid : 1;
} db_attr_t;

typedef union db_value {
  void* ptr;
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  char str[8];
} db_value_t;

typedef enum db_view_type { DB_VIEW_TYPE_SYS, DB_VIEW_TYPE_USR, DB_VIEW_TYPE_NUM } db_view_type_t;

struct db_object {
  db_value_t data;
  db_addr_t addr;
  db_attr_t attr;
  db_view_t viewer[DB_VIEW_TYPE_NUM];
};

struct db_object_arr {
  struct db_object objects[SPN_DB_MAX_OBJECT];
};

struct db_port {
  struct db_object_arr objects;
  int id;
};

struct db_interface {
  struct db_object_arr objects;
  struct db_port ports[SPN_DB_MAX_PORT];
  int id;
};

struct db_ctx {
  struct db_object_arr objects;
  struct db_interface interfaces[SPN_DB_MAX_INTERFACE];
};

static inline int db_is_static_object(struct db_object* object) {
  return !object->attr.is_dyn;
}

static inline int db_is_array_object(struct db_object* object) {
  return object->attr.is_arr;
}

static inline int db_is_valid_object(struct db_object* object) {
  return object->attr.is_valid;
}

static inline int db_is_static_string_object(struct db_object* object) {
  SPN_ASSERT("Invalid dynamic object",
             (object->attr.len > 8 && !db_is_static_object(object) && db_is_array_object(object)) ||
                 (object->attr.len <= 8 && db_is_static_object(object)));
  return db_is_static_object(object) && db_is_array_object(object) && object->attr.len < 8;
}

static inline unsigned db_object_len(struct db_object* object) {
  return object->attr.len;
}

/* TODO: Need implement this function to notify listener that object has been changed */
static inline void db_object_updated_ind(struct db_ctx* ctx, struct db_object* object, int flag) {
  ctx = ctx;
  object = object;
  flag = flag;
}

void db_init(struct db_ctx* ctx);
void db_deinit(struct db_ctx* ctx);

int db_add_interface(struct db_ctx* ctx, int interface_id);
int db_del_interface(struct db_interface* interface);
int db_dup_interface(struct db_interface* dst, struct db_interface* src);
int db_get_interface(struct db_ctx* ctx, int interface_id, struct db_interface** interface);

int db_add_port(struct db_interface* interface, int port_id);
int db_del_port(struct db_port* port);
int db_dup_port(struct db_port* dst, struct db_port* src);
int db_get_port(struct db_interface* interface, int port_id, struct db_port** port);

int db_add_object(struct db_object_arr* objects,
                  db_id_t id,
                  unsigned is_dynamic,
                  unsigned is_array,
                  size_t len,
                  db_value_t* data);
int db_del_object(struct db_object_arr* objects, db_id_t id);
int db_get_object(struct db_object_arr* objects, db_id_t id, struct db_object** object);
int db_dup_objects(struct db_object_arr* dst, struct db_object_arr* src);
void db_clear_objects(struct db_object_arr* objects);

/* Wrapper functions */
int db_get_global_object(struct db_ctx* ctx, db_id_t id, struct db_object** object);
int db_get_interface_object(struct db_ctx* ctx, int interface, db_id_t id, struct db_object** object);
int db_get_port_object(struct db_ctx* ctx, int interface, int port, db_id_t id, struct db_object** object);

#ifdef __cplusplus
}
#endif
