#include <spn/db_view.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <stdlib.h>
#include <string.h>

#if SPN_CONF_ASYNC
#error "Async is not supported"
#endif

static inline int get_idx_from_ptr(struct db_object* obj, struct db_object_arr* arr)
{
    ssize_t offset = (uintptr_t)obj - (uintptr_t)arr->objects;
    if (offset < 0 || (size_t)offset >= sizeof(arr->objects)) {
        return -1;
    }
    return offset / sizeof(*obj);
}

db_view_t db_view_create(struct db_object_arr* obj, db_view_cb_t callback)
{
    struct db_view* view;

    if (!obj || !callback) {
        return NULL;
    }
    view = (struct db_view*)malloc(sizeof(*view));
    if (!view) {
        return NULL;
    }
    memset(view, 0, sizeof(*view));
    view->objects = obj;
    view->callback = callback;
    return view;
}

void db_view_destroy(db_view_t view)
{
    struct db_view* v = (struct db_view*)view;
    if (!view) {
        return;
    }
#if !SPN_DEBUG
    /* for debug purpose */
    memset(v, 0, sizeof(*v));
#endif
    free(v);
}

int db_view_req(db_view_t view, struct db_object* object, int flag)
{
    struct db_view* v = (struct db_view*)view;
    int idx;
    SPN_ASSERT("view is invalid", v && v->objects);
    SPN_ASSERT("object is NULL", object);
    idx = get_idx_from_ptr(object, v->objects);
    SPN_ASSERT("object is not in the view", idx >= 0);
    /* set bit */
    v->inner[idx].state = DB_VIEW_STATE_REQ;
    v->inner[idx].val = flag;
    /* SYNC method, call db_view_ind directly */
    db_view_ind(view, object);
    return SPN_OK;
}

int db_view_cnf(db_view_t view, struct db_object* object, int* res)
{
    struct db_view* v = (struct db_view*)view;
    int idx;
    SPN_ASSERT("view is invalid", v && v->objects);
    SPN_ASSERT("object is NULL", object);
    idx = get_idx_from_ptr(object, v->objects);
    SPN_ASSERT("object is not in the view", idx >= 0);
    if (v->inner[idx].state == DB_VIEW_STATE_RES) {
        *res = v->inner[idx].val;
        v->inner[idx].state = DB_VIEW_STATE_IDLE;
        return SPN_OK;
    } else if (v->inner[idx].state == DB_VIEW_STATE_IDLE) {
        return -SPN_ENODATA;
    }
    return -SPN_EINPROGRESS;
}

int db_view_ind(db_view_t view, struct db_object* object)
{
    struct db_view* v = (struct db_view*)view;
    int idx;
    SPN_ASSERT("view is invalid", v && v->objects);
    SPN_ASSERT("object is NULL", object);
    SPN_ASSERT("callback is NULL", v->callback);
    idx = get_idx_from_ptr(object, v->objects);
    SPN_ASSERT("object is not in the view", idx >= 0);
    SPN_ASSERT("object is not requested", v->inner[idx].state == DB_VIEW_STATE_REQ);
    v->inner[idx].state = DB_VIEW_STATE_IND;
    v->callback(view, object, v->inner[idx].val);
    /* SYNC method, call db_view_res directly */
    return SPN_OK;
}

int db_view_rsp(db_view_t view, struct db_object* object, int ret)
{
    struct db_view* v = (struct db_view*)view;
    int idx;
    SPN_ASSERT("view is invalid", v && v->objects);
    SPN_ASSERT("object is NULL", object);
    idx = get_idx_from_ptr(object, v->objects);
    SPN_ASSERT("object is not in the view", idx >= 0);
    SPN_ASSERT("object is not indicated", v->inner[idx].state == DB_VIEW_STATE_IND);
    v->inner[idx].state = DB_VIEW_STATE_RES;
    v->inner[idx].val = ret;
    return SPN_OK;
}
