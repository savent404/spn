#include <spn/db.h>
#include <spn/db_view.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

static inline int find_idx_by_object(struct db_view* inst, struct db_object* obj) {
  unsigned idx;
  for (idx = 0; idx < ARRAY_SIZE(inst->objects); idx++) {
    if (inst->objects[idx] == obj) {
      return idx;
    }
  }
  return -1;
}

db_view_t db_view_init(struct db_view* instance, db_view_cb_t callback) {
  SPN_ASSERT("instance is null", instance != NULL);
  SPN_ASSERT("callback is null", callback != NULL);

  memset(instance, 0, sizeof(struct db_view));
  instance->callback = callback;

  return (db_view_t)instance;
}

int db_view_add_object(db_view_t v, enum db_view_type type, struct db_object* object) {
  struct db_view* inst = (struct db_view*)v;
  unsigned idx;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", object != NULL);

  if (object->viewer[type] != NULL) {
    return -SPN_EEXIST;
  }
  for (idx = 0; idx < ARRAY_SIZE(inst->objects); idx++) {
    if (inst->objects[idx] == NULL) {
      inst->objects[idx] = object;
      inst->attrs[idx].state = DB_VIEW_STATE_IDLE;
      inst->args[idx] = 0;
      object->viewer[type] = inst;
      return SPN_OK;
    }
  }
  return -SPN_ENOMEM;
}

int db_view_remove_object(db_view_t v, enum db_view_type type, struct db_object* object) {
  struct db_view* inst = (struct db_view*)v;
  int idx;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", object != NULL);

  if (object->viewer[type] != inst) {
    return -SPN_ENOENT;
  }
  idx = find_idx_by_object(inst, object);
  SPN_ASSERT("object not found", idx >= 0);
  inst->objects[idx] = NULL;
  object->viewer[type] = NULL;
  return SPN_OK;
}

void db_view_deinit(db_view_t v) {
  struct db_view* inst = (struct db_view*)v;
  unsigned idx;
  SPN_ASSERT("install is null", inst != NULL);

  for (idx = 0; idx < ARRAY_SIZE(inst->objects); idx++) {
    if (inst->objects[idx] == NULL) {
      continue;
    }
    if (inst->objects[idx]->viewer[DB_VIEW_TYPE_SYS] == inst) {
      inst->objects[idx]->viewer[DB_VIEW_TYPE_SYS] = NULL;
    } else if (inst->objects[idx]->viewer[DB_VIEW_TYPE_USR] == inst) {
      inst->objects[idx]->viewer[DB_VIEW_TYPE_USR] = NULL;
    } else {
      SPN_ASSERT("object not found", 0);
    }
  }
}

int db_view_req(db_view_t v, struct db_object* obj, uintptr_t arg) {
  struct db_view* inst = (struct db_view*)v;
  unsigned idx, cnt = 0;
  int res = -SPN_ENOENT;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", obj != NULL);

  for (idx = 0; idx < ARRAY_SIZE(inst->objects); idx++) {
    if (inst->objects[idx] == obj) {
      if (inst->attrs[idx].state != DB_VIEW_STATE_IDLE && inst->attrs[idx].state != DB_VIEW_STATE_RES) {
        res = -SPN_EBUSY;
        continue;
      }
      inst->attrs[idx].state = DB_VIEW_STATE_REQ;
      inst->args[idx] = arg;

      /* FIXME: View could be added in the same object by different type,
       * so we need to check all the objects in the view.
       * This is not a good design, but it's a workaround for now
       */
      res = db_view_ind(v, obj, idx);
      cnt++;
    }
  }

  return res;
}

int db_view_cnf(db_view_t v, struct db_object* obj, uintptr_t* res) {
  struct db_view* inst = (struct db_view*)v;
  int idx;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", obj != NULL);
  SPN_ASSERT("res is null", res != NULL);

  idx = find_idx_by_object(inst, obj);
  SPN_ASSERT("object not found", idx >= 0);

  if (inst->attrs[idx].state == DB_VIEW_STATE_IDLE) {
    return -SPN_EIO;
  }

  if (inst->attrs[idx].state != DB_VIEW_STATE_RES) {
    return -SPN_EBUSY;
  }
  *res = inst->args[idx];
  inst->attrs[idx].state = DB_VIEW_STATE_IDLE;

  return SPN_OK;
}

int db_view_ind(db_view_t v, struct db_object* obj, int idx) {
  struct db_view* inst = (struct db_view*)v;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", obj != NULL);

  if (inst->attrs[idx].state != DB_VIEW_STATE_REQ) {
    return -SPN_EIO;
  }
  inst->attrs[idx].state = DB_VIEW_STATE_IND;

  inst->callback(obj, inst->args[idx]);

  return SPN_OK;
}

int db_view_rsp(db_view_t v, struct db_object* obj, uintptr_t res) {
  struct db_view* inst = (struct db_view*)v;
  int idx;
  SPN_ASSERT("install is null", inst != NULL);
  SPN_ASSERT("object is null", obj != NULL);

  idx = find_idx_by_object(inst, obj);
  SPN_ASSERT("object not found", idx >= 0);

  if (inst->attrs[idx].state != DB_VIEW_STATE_IND) {
    return -SPN_EIO;
  }
  inst->attrs[idx].state = DB_VIEW_STATE_RES;
  inst->args[idx] = res;

  return SPN_OK;
}
