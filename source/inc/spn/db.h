#pragma once

#include <spn/errno.h>
#include <stddef.h>
#include <stdint.h>

#include <spn/db_ids.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DB_MAX_INTERFACE
#define DB_MAX_INTERFACE 2
#endif

#ifndef DB_MAX_PORT
#define DB_MAX_PORT 3
#endif

#ifndef DB_MAX_OBJECT
#define DB_MAX_OBJECT 16
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#pragma pack(push, 2)
typedef union db_value {
    void* ptr;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    char str[8];
} db_value_t;

struct db_object {
    struct {
        db_id_t id : 6;
        unsigned is_dynamic : 1;
        unsigned is_array : 1;
        unsigned len : 8; /* only for array */
    } header;
    db_value_t data;
};
#pragma pack(pop)

struct db_object_arr {
    struct db_object objects[DB_MAX_OBJECT];
};

struct db_port {
    int id;
    struct db_object_arr objects;
};

struct db_interface {
    int id;
    struct {
        unsigned is_outside : 1; /* Indicated that interface is outside of the device */
    } flags;
    struct db_port ports[DB_MAX_PORT];
    struct db_object_arr objects;
};

struct db_ctx {
    struct db_interface interfaces[DB_MAX_INTERFACE];
    struct db_object_arr objects;
};

static inline int db_is_static_object(struct db_object* object)
{
    return !object->header.is_dynamic;
}

static inline int db_is_array_object(struct db_object* object)
{
    return object->header.is_array;
}

static inline int db_is_static_string_object(struct db_object* object)
{
    return db_is_static_object(object) && db_is_array_object(object) && object->header.len < 8;
}

/* TODO: Need implement this function to notify listener that object has been changed */
static inline void db_object_updated_ind(struct db_ctx* ctx, struct db_object* object, int flag)
{
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

int db_add_object(struct db_object_arr* objects, db_id_t id, unsigned is_dynamic, unsigned is_array, size_t len, db_value_t* data);
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
