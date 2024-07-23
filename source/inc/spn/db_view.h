#pragma once

#include <spn/db.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum db_view_state {
  DB_VIEW_STATE_IDLE = 0x00,
  DB_VIEW_STATE_REQ = 0x01,
  DB_VIEW_STATE_IND = 0x02,
  DB_VIEW_STATE_RES = 0x03,
} db_view_state_t;

struct db_view_inner {
  enum db_view_state state : 2;
  int val : 30; /* restore input flag if state==req&ind, restore output val if state==resp&idle*/
};

struct db_view_attr {
  enum db_view_state state : 2;
  uint8_t reserved : 6;
};

typedef void (*db_view_cb_t)(struct db_object* object, uintptr_t arg, uint16_t interface, uint16_t port);

struct db_view {
  struct db_view_attr attrs[SPN_VIEW_MAX_OBJECT];
  uintptr_t args[SPN_VIEW_MAX_OBJECT];
  struct db_object* objects[SPN_VIEW_MAX_OBJECT];
  db_view_cb_t callback;
};

db_view_t db_view_init(struct db_view* instance, db_view_cb_t callback);
int db_view_add_object(db_view_t v, enum db_view_type type, struct db_object* object);
int db_view_remove_object(db_view_t v, enum db_view_type type, struct db_object* object);
void db_view_deinit(db_view_t v);

int db_view_req(db_view_t v, struct db_object* obj, uintptr_t arg);
int db_view_cnf(db_view_t v, struct db_object* obj, uintptr_t* res);

int db_view_ind(db_view_t v, struct db_object* obj, int idx);
int db_view_rsp(db_view_t v, struct db_object* obj, uintptr_t res);

#ifdef __cplusplus
}
#endif
