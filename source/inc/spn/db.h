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
  uint8_t mac[6];
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

/*@ axiomatic db_constants {
      logic integer DB_MAX_INTERFACE = SPN_DB_MAX_INTERFACE;
      logic integer DB_MAX_PORT = SPN_DB_MAX_PORT;
      logic integer DB_MAX_OBJECT = SPN_DB_MAX_OBJECT;
    }
*/

/*@ ghost int db_init_called; */

/*@
  requires \valid(ctx);
  assigns *ctx;
  behavior ok:
    ensures db_init_called == 1;
*/
void db_init(struct db_ctx* ctx);

/*@
  requires \valid(ctx);
  assigns *ctx;
*/
void db_deinit(struct db_ctx* ctx);

/*@
  requires \valid(ctx);
  requires \valid(ctx->interfaces + (0 .. SPN_DB_MAX_INTERFACE - 1));
  assigns ctx->interfaces[0 .. SPN_DB_MAX_INTERFACE - 1].id;
  ensures \result == 0 || \result == -12;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_INTERFACE && ctx->interfaces[i].id == -1;
    ensures \result == 0;
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_INTERFACE ==> ctx->interfaces[i].id != -1;
    ensures \result == -12;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_add_interface(struct db_ctx* ctx, int interface_id);

/*@
  requires \valid(interface);
  requires \valid(interface->ports + (0 .. SPN_DB_MAX_PORT - 1));
  assigns *interface, interface->ports[0 .. SPN_DB_MAX_PORT - 1];
  ensures \result == 0;
  ensures interface->id == -1;
*/
int db_del_interface(struct db_interface* interface);

/*@
  requires dst == \null || \valid(dst);
  requires src == \null || \valid(src);
  assigns \nothing;
  ensures \result == 0 || \result == -22 || \result == -12;
  behavior invalid_args:
    assumes dst == \null || src == \null || src->id == -1;
    ensures \result == -22;
*/
int db_dup_interface(struct db_interface* dst, struct db_interface* src);

/*@
  requires \valid(ctx);
  requires \valid(ctx->interfaces + (0 .. SPN_DB_MAX_INTERFACE - 1));
  requires \valid(interface);
  assigns *interface;
  ensures \result == 0 || \result == -2;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_INTERFACE && ctx->interfaces[i].id == interface_id;
    ensures \result == 0;
    ensures \valid(*interface);
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_INTERFACE ==> ctx->interfaces[i].id != interface_id;
    ensures \result == -2;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_get_interface(struct db_ctx* ctx, int interface_id, struct db_interface** interface);

/*@
  requires \valid(interface);
  requires \valid(interface->ports + (0 .. SPN_DB_MAX_PORT - 1));
  assigns interface->ports[0 .. SPN_DB_MAX_PORT - 1].id;
  ensures \result == 0 || \result == -12;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_PORT && interface->ports[i].id == -1;
    ensures \result == 0;
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_PORT ==> interface->ports[i].id != -1;
    ensures \result == -12;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_add_port(struct db_interface* interface, int port_id);

/*@
  requires \valid(port);
  assigns *port;
  ensures \result == 0;
  ensures port->id == -1;
*/
int db_del_port(struct db_port* port);

/*@
  requires dst == \null || \valid(dst);
  requires src == \null || \valid(src);
  assigns \nothing;
  ensures \result == 0 || \result == -22;
  behavior invalid_args:
    assumes dst == \null || src == \null || src->id == -1;
    ensures \result == -22;
*/
int db_dup_port(struct db_port* dst, struct db_port* src);

/*@
  requires \valid(interface);
  requires \valid(interface->ports + (0 .. SPN_DB_MAX_PORT - 1));
  requires \valid(port);
  assigns *port;
  ensures \result == 0 || \result == -2;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_PORT && interface->ports[i].id == port_id;
    ensures \result == 0;
    ensures \valid(*port);
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_PORT ==> interface->ports[i].id != port_id;
    ensures \result == -2;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_get_port(struct db_interface* interface, int port_id, struct db_port** port);

/*@
  requires \valid(objects);
  requires \valid(objects->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  requires \valid(data);
  requires len <= 255;
  assigns objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
  ensures \result == 0 || \result == -12;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_OBJECT && objects->objects[i].addr.obj == DB_ID_INVALID;
    ensures \result == 0;
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_OBJECT ==> objects->objects[i].addr.obj != DB_ID_INVALID;
    ensures \result == -12;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_add_object(struct db_object_arr* objects,
                  db_id_t id,
                  unsigned is_dynamic,
                  unsigned is_array,
                  size_t len,
                  db_value_t* data);

/*@
  requires \valid(objects);
  requires \valid(objects->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  assigns objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
  ensures \result == 0 || \result == -2;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_OBJECT && objects->objects[i].addr.obj == id;
    ensures \result == 0;
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_OBJECT ==> objects->objects[i].addr.obj != id;
    ensures \result == -2;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_del_object(struct db_object_arr* objects, db_id_t id);

/*@
  requires \valid(objects);
  requires \valid(objects->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  requires \valid(object);
  assigns *object;
  ensures \result == 0 || \result == -2;
  behavior found:
    assumes \exists integer i; 0 <= i < SPN_DB_MAX_OBJECT && objects->objects[i].addr.obj == id;
    ensures \result == 0;
    ensures \valid(*object);
  behavior not_found:
    assumes \forall integer i; 0 <= i < SPN_DB_MAX_OBJECT ==> objects->objects[i].addr.obj != id;
    ensures \result == -2;
  complete behaviors found, not_found;
  disjoint behaviors found, not_found;
*/
int db_get_object(struct db_object_arr* objects, db_id_t id, struct db_object** object);

/*@
  requires \valid(dst);
  requires \valid(src);
  requires \separated(dst, src);
  requires \valid(dst->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  requires \valid(src->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  assigns dst->objects[0 .. SPN_DB_MAX_OBJECT - 1];
  ensures \result == 0 || \result == -12;
*/
int db_dup_objects(struct db_object_arr* dst, struct db_object_arr* src);

/*@
  requires \valid(objects);
  requires \valid(objects->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  assigns objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
*/
void db_clear_objects(struct db_object_arr* objects);

/* Wrapper functions */

/*@
  requires \valid(ctx);
  requires \valid(ctx->objects.objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  requires \valid(object);
  assigns *object;
  ensures \result == 0 || \result == -2;
  ensures \result == 0 ==> \valid(*object);
*/
int db_get_global_object(struct db_ctx* ctx, db_id_t id, struct db_object** object);

/*@
  requires \valid(ctx);
  requires \valid(ctx->interfaces + (0 .. SPN_DB_MAX_INTERFACE - 1));
  requires \valid(object);
  assigns *object;
  ensures \result == 0 || \result == -2;
*/
int db_get_interface_object(struct db_ctx* ctx, int interface, db_id_t id, struct db_object** object);

/*@
  requires \valid(ctx);
  requires \valid(ctx->interfaces + (0 .. SPN_DB_MAX_INTERFACE - 1));
  requires \valid(object);
  assigns *object;
  ensures \result == 0 || \result == -2;
*/
int db_get_port_object(struct db_ctx* ctx, int interface, int port, db_id_t id, struct db_object** object);

#ifdef __cplusplus
}
#endif
