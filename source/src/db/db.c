#include <lwip/api.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <stdlib.h>

#include <string.h>

void db_init(struct db_ctx* ctx) {
  unsigned i, j;
  memset(ctx, 0, sizeof(*ctx));
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    struct db_interface* interface = &ctx->interfaces[i];
    interface->id = -1;
    for (j = 0; j < ARRAY_SIZE(interface->ports); j++) {
      interface->ports[j].id = -1;
    }
  }
}

void db_deinit(struct db_ctx* ctx) {
  unsigned i;
  db_clear_objects(&ctx->objects);
  for (i = 0; i < ARRAY_SIZE(ctx->interfaces); i++) {
    db_del_interface(&ctx->interfaces[i]);
  }
}

int db_add_interface(struct db_ctx* ctx, int interface_id) {
  unsigned i;
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
  for (i = 0; i < ARRAY_SIZE(interface->ports); i++) {
    if (interface->ports[i].id != -1) {
      db_del_port(&interface->ports[i]);
    }
  }
  db_clear_objects(&interface->objects);
  memset(interface, 0, sizeof(*interface));
  interface->id = -1;
  return SPN_OK;
}

int db_get_interface(struct db_ctx* ctx, int interface_id, struct db_interface** interface) {
  unsigned i;
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
  memset(port, 0, sizeof(*port));
  port->id = -1;
  return SPN_OK;
}

int db_get_port(struct db_interface* interface, int port_id, struct db_port** port) {
  unsigned i;
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
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    object = &objects->objects[i];
    if (object->attr.is_dyn) {
      SPN_ASSERT("object must be array", object->attr.is_arr && object->data.ptr != NULL);
      free(object->data.ptr);
      memset(object, 0, sizeof(*object));
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
  for (i = 0; i < ARRAY_SIZE(objects->objects); i++) {
    object = &objects->objects[i];
    if (object->addr.obj == id) {
      if (object->attr.is_dyn) {
        SPN_ASSERT("dynamic object must be array", object->attr.is_arr);
        free(object->data.ptr);
      }
      memset(object, 0, sizeof(*object));
      return SPN_OK;
    }
  }
  return -SPN_ENOENT;
}

int db_get_object(struct db_object_arr* objects, db_id_t id, struct db_object** object) {
  unsigned i;
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
