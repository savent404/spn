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

typedef void (*db_view_cb_t)(db_view_t view, struct db_object* object, int flag);

struct db_view {
    struct db_view_inner inner[SPN_DB_MAX_OBJECT];
    db_view_cb_t callback;
    struct db_object_arr* objects;
};

db_view_t db_view_create(struct db_object_arr* objs, db_view_cb_t callback);
void db_view_destroy(db_view_t view);

int db_view_req(db_view_t view, struct db_object* object, int flag);
int db_view_cnf(db_view_t view, struct db_object* object, int* res);
int db_view_ind(db_view_t view, struct db_object* object);
int db_view_rsp(db_view_t view, struct db_object* object, int ret);

#ifdef __cplusplus
}
#endif
