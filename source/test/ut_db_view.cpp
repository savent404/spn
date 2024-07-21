#include <gtest/gtest.h>
#include <spn/db_view.h>
#include <spn/errno.h>
#include <functional>

namespace {
struct _db_view {
  using callback_t = std::function<void(struct db_object*, uintptr_t)>;

  static _db_view* get_instance() {
    static _db_view instance;
    return &instance;
  }

  static void callback_wrapper(struct db_object* object, uintptr_t arg) { get_instance()->callback(object, arg); }

  void callback(struct db_object* object, uintptr_t arg) {
    if (callback_) {
      callback_(object, arg);
    }
  }

  callback_t callback_;
};

struct View : public testing::Test {
  void SetUp() override {
    _db_view::get_instance()->callback_ = nullptr;
    db_init(&ctx);
    db_view_init(&view, _db_view::callback_wrapper);
  }

  void TearDown() override {
    _db_view::get_instance()->callback_ = nullptr;
    db_deinit(&ctx);
    db_view_deinit(&view);
  }

  struct db_ctx ctx;
  struct db_view view;
};
}  // namespace

TEST_F(View, db_view_init) {
  struct db_view view;
  db_view_init(&view, _db_view::callback_wrapper);
  EXPECT_EQ(view.callback, _db_view::callback_wrapper);
}

TEST_F(View, add_remove_object) {
  auto& object = ctx.objects.objects[0];
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);

  db_view_remove_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], nullptr);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], nullptr);
}

TEST_F(View, add_object) {
  auto& object = ctx.objects.objects[0];
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);

  db_view_add_object(&view, DB_VIEW_TYPE_USR, &object);
  EXPECT_EQ(view.objects[1], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_USR], &view);
}

TEST_F(View, view_dup_type) {
  auto& object = ctx.objects.objects[0];
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), SPN_OK);
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), -SPN_EEXIST);
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_USR, &object), SPN_OK);
}

TEST_F(View, notify_dup_type) {
  auto& object = ctx.objects.objects[0];
  int called = 0;
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), SPN_OK);
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_USR, &object), SPN_OK);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    EXPECT_EQ(object, &ctx.objects.objects[0]);
    EXPECT_EQ(arg, 0x1234);
    called += 1;
  };

  EXPECT_EQ(db_view_req(&view, &object, 0x1234), SPN_OK);
  EXPECT_EQ(called, 2);
}

TEST_F(View, remove_dup_type) {
  auto& object = ctx.objects.objects[0];
  int called = 0;
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), SPN_OK);
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_USR, &object), SPN_OK);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_USR], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    EXPECT_EQ(object, &ctx.objects.objects[0]);
    EXPECT_EQ(arg, 0x1234);
    called += 1;
  };

  EXPECT_EQ(db_view_remove_object(&view, DB_VIEW_TYPE_USR, &object), SPN_OK);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_USR], nullptr);

  EXPECT_EQ(db_view_req(&view, &object, 0x1234), SPN_OK);
  EXPECT_EQ(called, 1);

  EXPECT_EQ(db_view_remove_object(&view, DB_VIEW_TYPE_SYS, &object), SPN_OK);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], nullptr);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_USR], nullptr);
  EXPECT_EQ(db_view_req(&view, &object, 0x1234), -SPN_ENOENT);
  EXPECT_EQ(called, 1);
}

TEST_F(View, add_object_twice) {
  auto& object = ctx.objects.objects[0];
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), -SPN_EEXIST);
}

TEST_F(View, remove_object_twice) {
  auto& object = ctx.objects.objects[0];
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);
  db_view_remove_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], nullptr);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], nullptr);
  EXPECT_EQ(db_view_remove_object(&view, DB_VIEW_TYPE_SYS, &object), -SPN_ENOENT);
}

TEST_F(View, notify) {
  auto& object = ctx.objects.objects[0];
  bool called = false;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    EXPECT_EQ(object, &ctx.objects.objects[0]);
    EXPECT_EQ(arg, 0x1234);
    called = true;
  };

  db_view_req(&view, &object, 0x1234);
  EXPECT_TRUE(called);
}

TEST_F(View, notify_multiple) {
  auto& object1 = ctx.objects.objects[0];
  auto& object2 = ctx.objects.objects[1];
  bool called1 = false;
  bool called2 = false;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object1);
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object2);
  EXPECT_EQ(view.objects[0], &object1);
  EXPECT_EQ(view.objects[1], &object2);
  EXPECT_EQ(object1.viewer[DB_VIEW_TYPE_SYS], &view);
  EXPECT_EQ(object2.viewer[DB_VIEW_TYPE_SYS], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    if (object == &ctx.objects.objects[0]) {
      EXPECT_EQ(arg, 0x1234);
      called1 = true;
    } else if (object == &ctx.objects.objects[1]) {
      EXPECT_EQ(arg, 0x5678);
      called2 = true;
    }
  };

  db_view_req(&view, &object1, 0x1234);
  db_view_req(&view, &object2, 0x5678);
  EXPECT_TRUE(called1);
  EXPECT_TRUE(called2);
}

TEST_F(View, notify_and_confirm) {
  auto& object = ctx.objects.objects[0];
  bool called = false;
  bool confirmed = false;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    EXPECT_EQ(object, &ctx.objects.objects[0]);
    EXPECT_EQ(arg, 0x1234);
    called = true;

    db_view_rsp(&view, object, 0x4321);
  };

  db_view_req(&view, &object, 0x1234);
  EXPECT_TRUE(called);

  uintptr_t res;
  EXPECT_EQ(db_view_cnf(&view, &object, &res), SPN_OK);
  EXPECT_EQ(res, 0x4321);
}

TEST_F(View, notify_and_confirm_multiple) {
  auto& object1 = ctx.objects.objects[0];
  auto& object2 = ctx.objects.objects[1];
  bool called1 = false;
  bool called2 = false;
  bool confirmed1 = false;
  bool confirmed2 = false;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object1);
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object2);
  EXPECT_EQ(view.objects[0], &object1);
  EXPECT_EQ(view.objects[1], &object2);
  EXPECT_EQ(object1.viewer[DB_VIEW_TYPE_SYS], &view);
  EXPECT_EQ(object2.viewer[DB_VIEW_TYPE_SYS], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    if (object == &ctx.objects.objects[0]) {
      EXPECT_EQ(arg, 0x1234);
      called1 = true;

      db_view_rsp(&view, object, 0x4321);
    } else if (object == &ctx.objects.objects[1]) {
      EXPECT_EQ(arg, 0x5678);
      called2 = true;

      db_view_rsp(&view, object, 0x8765);
    }
  };

  db_view_req(&view, &object1, 0x1234);
  db_view_req(&view, &object2, 0x5678);
  EXPECT_TRUE(called1);
  EXPECT_TRUE(called2);

  uintptr_t res;
  EXPECT_EQ(db_view_cnf(&view, &object1, &res), SPN_OK);
  EXPECT_EQ(res, 0x4321);
  EXPECT_EQ(db_view_cnf(&view, &object2, &res), SPN_OK);
  EXPECT_EQ(res, 0x8765);
}

TEST_F(View, add_many_objects) {
  for (int i = 0; i < SPN_VIEW_MAX_OBJECT; i++) {
    auto& object = ctx.objects.objects[i];
    db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
    EXPECT_EQ(view.objects[i], &object);
    EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);
  }

  auto& object = ctx.objects.objects[SPN_VIEW_MAX_OBJECT];
  EXPECT_EQ(db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object), -SPN_ENOMEM);
}

TEST_F(View, notify_twice) {
  auto& object = ctx.objects.objects[0];
  bool called = false;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(view.objects[0], &object);
  EXPECT_EQ(object.viewer[DB_VIEW_TYPE_SYS], &view);

  _db_view::get_instance()->callback_ = [&](struct db_object* object, uintptr_t arg) {
    EXPECT_EQ(object, &ctx.objects.objects[0]);
    EXPECT_EQ(arg, 0x1234);
    called = true;
  };

  EXPECT_EQ(db_view_req(&view, &object, 0x1234), SPN_OK);
  EXPECT_TRUE(called);

  called = false;
  EXPECT_EQ(db_view_req(&view, &object, 0x5678), -SPN_EBUSY);
  EXPECT_FALSE(called);
}

TEST_F(View, cnf_without_ind) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(db_view_cnf(&view, &object, &res), -SPN_EIO);
}

TEST_F(View, cnf_without_ind_2) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  view.attrs[0].state = DB_VIEW_STATE_REQ;
  EXPECT_EQ(db_view_cnf(&view, &object, &res), -SPN_EBUSY);
}

TEST_F(View, ind_without_req) {
  auto& object = ctx.objects.objects[0];
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(db_view_ind(&view, &object, 0), -SPN_EIO);
}

TEST_F(View, cnf_without_rsp) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  db_view_req(&view, &object, 0x1234);
  EXPECT_EQ(db_view_cnf(&view, &object, &res), -SPN_EBUSY);
}

TEST_F(View, rsp_without_req) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(db_view_rsp(&view, &object, 0x1234), -SPN_EIO);
}

TEST_F(View, req_already_dead) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  db_view_remove_object(&view, DB_VIEW_TYPE_SYS, &object);
  EXPECT_EQ(db_view_req(&view, &object, 0x1234), -SPN_ENOENT);
}

TEST_F(View, deinit_already_craped) {
  auto& object = ctx.objects.objects[0];
  uintptr_t res;
  db_view_add_object(&view, DB_VIEW_TYPE_SYS, &object);
  object.viewer[DB_VIEW_TYPE_SYS] = nullptr;
  EXPECT_DEATH(db_view_deinit(&view), ".*");
  memset(&view, 0, sizeof(view));
}
