#include <functional>
#include <gtest/gtest.h>
#include <spn/db_view.h>
#include <spn/errno.h>

using view_cb_t = std::function<int(db_view_t, struct db_object*, int)>;
static view_cb_t cb_instance = nullptr;

static void cb_wrapper(db_view_t view, struct db_object* object, int flag)
{
    int res = cb_instance(view, object, flag);
    db_view_res(view, object, res);
}

TEST(db_view, init)
{
    struct db_object_arr obj;
    auto view = db_view_create(&obj, cb_wrapper);
    db_view_destroy(view);
}

TEST(db_view, mem_leak)
{
    struct db_object_arr obj;
    for (auto i = 0; i < 1000; i++) {
        auto view = db_view_create(&obj, cb_wrapper);
        db_view_destroy(view);
    }
}

TEST(db_view, sync_req)
{
    struct db_object_arr obj = { 0 };
    struct db_object* o;
    db_value_t val;

    ASSERT_EQ(db_add_object(&obj, (db_id_t)(DB_ID_INVALID + 1), 0, 0, 1, &val), SPN_OK);
    o = &obj.objects[0];

    cb_instance = [&](db_view_t view, struct db_object* object, int flag) {
        return flag;
    };
    auto view = db_view_create(&obj, cb_wrapper);

    for (int i = -4096; i < 4096; i++) {
        int res;
        ASSERT_EQ(db_view_req(view, o, i), SPN_OK);
        ASSERT_EQ(db_view_cnf(view, o, &res), SPN_OK);
        ASSERT_EQ(res, i);
    }

    db_view_destroy(view);
}

TEST(db_view, invalid_obj)
{
    struct db_object_arr obj = { 0 };
    struct db_object_arr obj2 = { 0 };
    struct db_object* o;
    db_value_t val;

    ASSERT_EQ(db_add_object(&obj, (db_id_t)(DB_ID_INVALID + 1), 0, 0, 1, &val), SPN_OK);
    o = &obj2.objects[0];

    cb_instance = [&](db_view_t view, struct db_object* object, int flag) {
        return flag;
    };
    auto view = db_view_create(&obj, cb_wrapper);
    ASSERT_EXIT(db_view_req(view, o, 10), ::testing::KilledBySignal(SIGABRT), "");
    ASSERT_DEATH(db_view_req(view, o, 10), "");

    db_view_destroy(view);
}