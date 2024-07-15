#include <lwip/api.h>
#include <spn/db.h>
#include <stdlib.h>

#include <string.h>

void db_init(struct db_ctx* ctx)
{
    unsigned i, j;
    memset(ctx, 0, sizeof(*ctx));

    for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
        ctx->interfaces[i].id = -1;
        for (j = 0; j < ARRAY_SIZE(ctx->interfaces[i].ports); j++) {
            ctx->interfaces[i].ports[j].id = -1;
        }
    }
}

void db_deinit(struct db_ctx* ctx)
{
    unsigned i, j;
    db_clear_objects(&ctx->objects);
    for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
        db_clear_objects(&ctx->interfaces[i].objects);
        for (j = 0; j < ARRAY_SIZE(ctx->interfaces[i].ports); j++) {
            db_clear_objects(&ctx->interfaces[i].ports[j].objects);
        }
    }
}

int db_add_interface(struct db_ctx* ctx, int interface_id)
{
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
        if (ctx->interfaces[i].id == -1) {
            ctx->interfaces[i].id = interface_id;
            return SPN_OK;
        }
    }
    return -SPN_ENOMEM;
}

int db_get_interface(struct db_ctx* ctx, int interface_id, struct db_interface** interface)
{
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
        if (ctx->interfaces[i].id == interface_id) {
            *interface = &ctx->interfaces[i];
            return SPN_OK;
        }
    }
    return -SPN_ENOENT;
}

int db_add_port(struct db_interface* interface, int port_id)
{
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
        if (interface->ports[i].id == -1) {
            interface->ports[i].id = port_id;
            return SPN_OK;
        }
    }
    return -SPN_ENOMEM;
}

int db_get_port(struct db_interface* interface, int port_id, struct db_port** port)
{
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
        if (interface->ports[i].id == port_id) {
            *port = &interface->ports[i];
            return SPN_OK;
        }
    }
    return -SPN_ENOENT;
}

void db_clear_objects(struct db_object_arr* objects)
{
    struct db_object* object;
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
        object = &objects->objects[i];
        if (object->header.is_dynamic) {
            LWIP_ASSERT("object must be array", object->header.is_array && object->data.ptr != NULL);
            free(object->data.ptr);
            memset(object, 0, sizeof(*object));
        }
    }
}

int db_add_object(struct db_object_arr* objects, db_id_t id, unsigned is_dynamic, unsigned is_array, size_t len, db_value_t* data)
{
    struct db_object* object;
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
        object = &objects->objects[i];
        if (object->header.id == DB_ID_INVALID) {
            object->header.id = id;
            object->header.is_dynamic = is_dynamic;
            object->header.is_array = is_array;
            object->header.len = len;
            LWIP_ASSERT("dynamic object must be array", !is_dynamic || is_array);
            LWIP_ASSERT("dynamic object must have data", !is_dynamic || data->ptr != NULL);
            object->data = *data;
            return SPN_OK;
        }
    }
    LWIP_ASSERT("no free object", 0);
    return -SPN_ENOMEM;
}

int db_del_object(struct db_object_arr* objects, db_id_t id)
{
    struct db_object* object;
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
        object = &objects->objects[i];
        if (object->header.id == id) {
            if (object->header.is_dynamic) {
                LWIP_ASSERT("dynamic object must be array", object->header.is_array);
                free(object->data.ptr);
            }
            memset(object, 0, sizeof(*object));
            return SPN_OK;
        }
    }
    return -SPN_ENOENT;
}

int db_get_object(struct db_object_arr* objects, db_id_t id, struct db_object** object)
{
    unsigned i;
    for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
        if (objects->objects[i].header.id == id) {
            *object = &objects->objects[i];
            return SPN_OK;
        }
    }
    return -SPN_ENOENT;
}

int db_get_global_object(struct db_ctx* ctx, db_id_t id, struct db_object** object)
{
    return db_get_object(&ctx->objects, id, object);
}

int db_get_interface_object(struct db_ctx* ctx, int interface, db_id_t id, struct db_object** object)
{
    struct db_interface* intf;
    int err = db_get_interface(ctx, interface, &intf);
    if (err < 0) {
        return err;
    }
    return db_get_object(&intf->objects, id, object);
}

int db_get_port_object(struct db_ctx* ctx, int interface, int port, db_id_t id, struct db_object** object)
{
    struct db_interface* intf;
    struct db_port* p;
    int err = db_get_interface(ctx, interface, &intf);
    if (err < 0) {
        return err;
    }
    err = db_get_port(intf, port, &p);
    if (err < 0) {
        return err;
    }
    return db_get_object(&p->objects, id, object);
}
