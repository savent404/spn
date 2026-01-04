#include <lwip/api.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <stdlib.h>

#include <string.h>

/*@
  requires \valid(object);
  assigns *object;
  ensures object->addr.obj == DB_ID_INVALID;
  ensures object->addr.iface == 0;
  ensures object->addr.port == 0;
  ensures object->attr.len == 0;
  ensures object->attr.is_dyn == 0;
  ensures object->attr.is_arr == 0;
  ensures object->attr.is_valid == 0;
*/
static void db_object_init(struct db_object* object) {
  object->data.u64 = 0;
  object->addr.obj = DB_ID_INVALID;
  object->addr.iface = 0;
  object->addr.port = 0;
  object->attr.len = 0;
  object->attr.is_dyn = 0;
  object->attr.is_arr = 0;
  object->attr.is_valid = 0;
  object->viewer[0] = (db_view_t)0;
  object->viewer[1] = (db_view_t)0;
}

/*@
  requires \valid(objects);
  requires \valid(objects->objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  assigns objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
*/
static void db_object_arr_init(struct db_object_arr* objects) {
  unsigned k;
  /*@
    loop invariant 0 <= k <= SPN_DB_MAX_OBJECT;
    loop assigns k, objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
    loop variant SPN_DB_MAX_OBJECT - k;
  */
  for (k = 0; k < SPN_DB_MAX_OBJECT; k++) {
    db_object_init(&objects->objects[k]);
  }
}

/*@
  requires \valid(port);
  requires \valid(port->objects.objects + (0 .. SPN_DB_MAX_OBJECT - 1));
  assigns *port;
  ensures port->id == -1;
*/
static void db_port_init(struct db_port* port) {
  port->id = -1;
  db_object_arr_init(&port->objects);
}

/*@
  requires \valid(interface);
  requires \valid(interface->ports + (0 .. SPN_DB_MAX_PORT - 1));
  assigns *interface;
  ensures interface->id == -1;
*/
static void db_interface_init(struct db_interface* interface) {
  unsigned j;
  interface->id = -1;
  db_object_arr_init(&interface->objects);
  /*@
    loop invariant 0 <= j <= SPN_DB_MAX_PORT;
    loop assigns j, interface->ports[0 .. SPN_DB_MAX_PORT - 1];
    loop variant SPN_DB_MAX_PORT - j;
  */
  for (j = 0; j < SPN_DB_MAX_PORT; j++) {
    db_port_init(&interface->ports[j]);
  }
}

void db_init(struct db_ctx* ctx) {
  unsigned i;
  db_object_arr_init(&ctx->objects);
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_INTERFACE;
    loop assigns i, ctx->interfaces[0 .. SPN_DB_MAX_INTERFACE - 1];
    loop variant SPN_DB_MAX_INTERFACE - i;
  */
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    db_interface_init(&ctx->interfaces[i]);
  }
}

void db_deinit(struct db_ctx* ctx) {
  unsigned i;
  db_clear_objects(&ctx->objects);
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_INTERFACE;
    loop assigns i, ctx->interfaces[0 .. SPN_DB_MAX_INTERFACE - 1];
    loop variant SPN_DB_MAX_INTERFACE - i;
  */
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    db_del_interface(&ctx->interfaces[i]);
  }
}

int db_add_interface(struct db_ctx* ctx, int interface_id) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_INTERFACE;
    loop assigns i;
    loop variant SPN_DB_MAX_INTERFACE - i;
  */
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    if (ctx->interfaces[i].id == -1) {
      ctx->interfaces[i].id = interface_id;
      return SPN_OK;
    }
  }
  return -SPN_ENOMEM;
}

int db_del_interface(struct db_interface* interface) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_PORT;
    loop assigns i, interface->ports[0 .. SPN_DB_MAX_PORT - 1];
    loop variant SPN_DB_MAX_PORT - i;
  */
  for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
    if (interface->ports[i].id != -1) {
      db_del_port(&interface->ports[i]);
    }
  }
  db_clear_objects(&interface->objects);
  db_interface_init(interface);
  return SPN_OK;
}

int db_get_interface(struct db_ctx* ctx, int interface_id, struct db_interface** interface) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_INTERFACE;
    loop assigns i;
    loop variant SPN_DB_MAX_INTERFACE - i;
  */
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    if (ctx->interfaces[i].id == interface_id) {
      *interface = &ctx->interfaces[i];
      return SPN_OK;
    }
  }
  return -SPN_ENOENT;
}

int db_dup_interface(struct db_interface* dst, struct db_interface* src) {
  int res;
  unsigned i;
  if (!dst || !src || src->id == -1) {
    return -SPN_EINVAL;
  }

  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_PORT;
    loop assigns i, res, dst->ports[0 .. SPN_DB_MAX_PORT - 1];
    loop variant SPN_DB_MAX_PORT - i;
  */
  for (i = 0; i < ARRAY_SIZE(dst->ports); i++) {
    res = db_dup_port(&dst->ports[i], &src->ports[i]);
    if (res != SPN_OK) {
      return res;
    }
  }
  db_clear_objects(&dst->objects);
  db_dup_objects(&dst->objects, &src->objects);
  return SPN_OK;
}

int db_add_port(struct db_interface* interface, int port_id) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_PORT;
    loop assigns i;
    loop variant SPN_DB_MAX_PORT - i;
  */
  for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
    if (interface->ports[i].id == -1) {
      interface->ports[i].id = port_id;
      return SPN_OK;
    }
  }
  return -SPN_ENOMEM;
}

int db_del_port(struct db_port* port) {
  db_clear_objects(&port->objects);
  db_port_init(port);
  return SPN_OK;
}

int db_get_port(struct db_interface* interface, int port_id, struct db_port** port) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_PORT;
    loop assigns i;
    loop variant SPN_DB_MAX_PORT - i;
  */
  for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
    if (interface->ports[i].id == port_id) {
      *port = &interface->ports[i];
      return SPN_OK;
    }
  }
  return -SPN_ENOENT;
}

int db_dup_port(struct db_port* dst, struct db_port* src) {
  int res;
  if (!dst || !src || src->id == -1) {
    return -SPN_EINVAL;
  }
  res = db_del_port(dst);
  if (res < 0) {
    return res;
  }
  res = db_dup_objects(&dst->objects, &src->objects);
  return SPN_OK;
}

int db_dup_objects(struct db_object_arr* dst, struct db_object_arr* src) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_OBJECT;
    loop assigns i, dst->objects[0 .. SPN_DB_MAX_OBJECT - 1];
    loop variant SPN_DB_MAX_OBJECT - i;
  */
  for (i = 0; i < ARRAY_SIZE(dst->objects); i++) {
    if (src->objects[i].addr.obj != DB_ID_INVALID) {
      struct db_object* src_obj = &src->objects[i];
      int res;
      if (src_obj->attr.is_dyn) {
        db_value_t v;

        v.ptr = malloc(src_obj->attr.len);
        if (!v.ptr) {
          goto err_ret;
        }
        memcpy(v.ptr, src_obj->data.ptr, src_obj->attr.len);
        res = db_add_object(dst, src_obj->addr.obj, src_obj->attr.is_dyn, src_obj->attr.is_arr,
                            src_obj->attr.len, &v);
      } else {
        res = db_add_object(dst, src_obj->addr.obj, src_obj->attr.is_dyn, src_obj->attr.is_arr,
                            src_obj->attr.len, &src_obj->data);
      }

      if (res < 0) {
        return res;
      }
    }
  }
  return SPN_OK;
err_ret:
  db_clear_objects(dst);
  return -SPN_ENOMEM;
}

void db_clear_objects(struct db_object_arr* objects) {
  struct db_object* object;
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_OBJECT;
    loop assigns i, objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
    loop variant SPN_DB_MAX_OBJECT - i;
  */
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    object = &objects->objects[i];
    if (object->attr.is_dyn) {
      SPN_ASSERT("object must be array", object->attr.is_arr && object->data.ptr != NULL);
      free(object->data.ptr);
      db_object_init(object);
    }
  }
}

int db_add_object(struct db_object_arr* objects,
                  db_id_t id,
                  unsigned is_dyn,
                  unsigned is_arr,
                  size_t len,
                  db_value_t* data) {
  struct db_object* object;
  unsigned i;
  SPN_ASSERT("invalid object size", len <= 255);
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_OBJECT;
    loop assigns i, object;
    loop variant SPN_DB_MAX_OBJECT - i;
  */
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    object = &objects->objects[i];
    if (object->addr.obj == DB_ID_INVALID) {
      /* TODO: set addr.interface and addr.port also */
      object->addr.obj = id;
      object->attr.is_dyn = is_dyn;
      object->attr.is_arr = is_arr;
      object->attr.len = len;
      SPN_ASSERT("dynamic object must be array", !is_dyn || is_arr);
      SPN_ASSERT("dynamic object must have data", !is_dyn || data->ptr != NULL);
      object->data = *data;
      return SPN_OK;
    }
  }
  SPN_ASSERT("no free object", 0);
  return -SPN_ENOMEM;
}

int db_del_object(struct db_object_arr* objects, db_id_t id) {
  struct db_object* object;
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_OBJECT;
    loop assigns i, object, objects->objects[0 .. SPN_DB_MAX_OBJECT - 1];
    loop variant SPN_DB_MAX_OBJECT - i;
  */
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    object = &objects->objects[i];
    if (object->addr.obj == id) {
      if (object->attr.is_dyn) {
        SPN_ASSERT("dynamic object must be array", object->attr.is_arr);
        free(object->data.ptr);
      }
      db_object_init(object);
      return SPN_OK;
    }
  }
  return -SPN_ENOENT;
}

int db_get_object(struct db_object_arr* objects, db_id_t id, struct db_object** object) {
  unsigned i;
  /*@
    loop invariant 0 <= i <= SPN_DB_MAX_OBJECT;
    loop assigns i;
    loop variant SPN_DB_MAX_OBJECT - i;
  */
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    if (objects->objects[i].addr.obj == id) {
      *object = &objects->objects[i];
      return SPN_OK;
    }
  }
  return -SPN_ENOENT;
}

int db_get_global_object(struct db_ctx* ctx, db_id_t id, struct db_object** object) {
  return db_get_object(&ctx->objects, id, object);
}

int db_get_interface_object(struct db_ctx* ctx, int interface, db_id_t id, struct db_object** object) {
  struct db_interface* intf;
  int err = db_get_interface(ctx, interface, &intf);
  if (err < 0) {
    return err;
  }
  return db_get_object(&intf->objects, id, object);
}

int db_get_port_object(struct db_ctx* ctx, int interface, int port, db_id_t id, struct db_object** object) {
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
