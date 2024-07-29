#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include "dcp_ctx.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct DcpGet : public ::testing::Test {
  dcp_inst_ptr controller;
  dcp_inst_ptr device;

  void SetUp() override {
    controller = std::make_shared<dcp_instance>();
    device = std::make_shared<dcp_instance>();
    controller->db_setup();
    device->db_setup();
    device->db_info_setup(0, 0x0a000001, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, "device", "iod", 0x1234, 0x5678, 0x01);
    controller->db_info_setup(0, 0x0a000002, {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, "controller", "ioc", 0x1234, 0x5678,
                              0x02);

    // assume that controller find device by DCP.ident
    char buf[1500];
    uint16_t buf_len;
    controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
    dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len);
    dcp_srv_ident_ind(&device->dcp, &device->dcp.mcr_ctx[0], buf + 2, buf_len - 2);
    dcp_srv_ident_rsp(&device->dcp, &device->dcp.mcr_ctx[0], buf, &buf_len);
    dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, NULL);
  }

  void TearDown() override {
    controller.reset();
    device.reset();
  }
};
}  // namespace

TEST_F(DcpGet, GetNameOfStation) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  // overwrite the name of station, so we can check if it is changed to the correct value
  struct db_object* obj;
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj),
            SPN_OK);
  db_dup_str2obj(obj, "dummy", 5);


  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
  ASSERT_EQ(dcp_srv_get_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_get_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_get_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_get_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj),
            SPN_OK);
  EXPECT_EQ(db_object_len(obj), 6);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "device", 6), 0);
}

TEST_F(DcpGet, GetNameOfStation_timeout) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;
  struct db_object* obj;

  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
  ASSERT_EQ(dcp_srv_get_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  _dcp_srv_get_req_timeout(&controller->dcp.ucs_ctx);
  ASSERT_EQ(dcp_srv_get_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // check the result
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj),
            SPN_OK);
  EXPECT_EQ(db_object_len(obj), 6);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "device", 6), 0);
}
