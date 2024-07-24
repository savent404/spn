#pragma once

#include <spn/db.h>

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
