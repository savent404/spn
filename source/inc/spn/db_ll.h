#pragma once

#include <spn/db.h>
#include <stdlib.h>
#include <string.h>

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

static inline int db_cmp_str2obj(struct db_object* str1, const char* str2, unsigned len) {
  if (db_is_static_string_object(str1)) {
    return strncmp(str1->data.str, str2, len);
  } else {
    return strncmp((const char*)str1->data.ptr, str2, len);
  }
}

static inline int db_dup_str2obj(struct db_object* dst, const char* src, unsigned len) {
  if (len < sizeof(dst->data.str)) {
    memcpy(dst->data.str, src, len);
    dst->attr.is_dyn = 0;
    dst->attr.len = len;
  } else {
    dst->data.ptr = malloc(len);
    if (!dst->data.ptr) {
      return -1;
    }
    memcpy(dst->data.ptr, src, len);
    dst->attr.is_dyn = 1;
    dst->attr.len = len;
  }
  dst->attr.is_arr = 1;
  return 0;
}

static inline int db_free_objstr(struct db_object* object) {
  if (!db_is_static_object(object)) {
    free(object->data.ptr);
  }
  object->attr.is_dyn = 0;
  object->attr.len = 0;
  return 0;
}

static inline int db_strcpy_obj2str(void* dst, struct db_object* src) {
  unsigned len = db_object_len(src);
  if (db_is_static_string_object(src)) {
    memcpy(dst, src->data.str, len);
  } else {
    memcpy((char*)dst, src->data.ptr, len);
  }
  return len;
}
