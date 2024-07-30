#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include "dcp_ctx.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct DcpSet : public ::testing::Test {
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
    controller->dcp.ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
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

void tain_buffer(char* buf, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    buf[i] = i;
  }
}

}  // namespace

TEST_F(DcpSet, SetNameOfStation) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "fooboo";
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  struct db_object* obj;
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_NAME_OF_INTERFACE, &obj), SPN_OK);
  EXPECT_EQ(db_object_len(obj), 6);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "fooboo", 6), 0);

  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj),
            SPN_OK);
  EXPECT_EQ(db_object_len(obj), 6);
  EXPECT_EQ(strncmp(obj->data.str, "fooboo", 6), 0);
}

TEST_F(DcpSet, SetNameOfStation_odd) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  /* NOTE: Use odd number of string to test 2-bytes align padding also */
  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "fooboo1";
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  struct db_object* obj;
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_NAME_OF_INTERFACE, &obj), SPN_OK);
  EXPECT_EQ(db_object_len(obj), 7);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "fooboo1", 7), 0);
}
TEST_F(DcpSet, SetNameOfStation_timeout) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "fooboo";
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  _dcp_srv_set_req_timeout(&controller->dcp.ucs_ctx);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), -SPN_EAGAIN);
}

TEST_F(DcpSet, SetNameOfStation_uppercase) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  /* NOTE: Use odd number of string to test 2-bytes align padding also */
  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "Fooboo1";
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  EXPECT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION], DCP_BLOCK_ERR_RESOURCE_ERR);
}

TEST_F(DcpSet, SetIpParam) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.ucs_ctx.ip_addr = 0x0a000003;
  controller->dcp.ucs_ctx.ip_mask = 0x0a000004;
  controller->dcp.ucs_ctx.ip_gw = 0x0a000005;
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  struct db_object* obj;
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_IP_ADDR, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000003);
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_IP_MASK, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000004);
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_IP_GATEWAY, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000005);

  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_ADDR, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000003);
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MASK, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000004);
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_GATEWAY, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0x0a000005);
}

TEST_F(DcpSet, SetIpParam_invalid_length) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.ucs_ctx.ip_addr = 0x0a000003;
  controller->dcp.ucs_ctx.ip_mask = 0x0a000004;
  controller->dcp.ucs_ctx.ip_gw = 0x0a000005;
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  buf[12 + 3] = 0xFF;
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);

  // check the result
  EXPECT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_IP_PARAMETER], DCP_BLOCK_ERR_RESOURCE_ERR);
}

TEST_F(DcpSet, SetCtrl) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_CTRL_START | 1 << DCP_BIT_IDX_CTRL_STOP |
                                               1 << DCP_BIT_IDX_CTRL_SIGNAL | 1 << DCP_BIT_IDX_CTRL_FACTORY_RESET |
                                               1 << DCP_BIT_IDX_CTRL_RESET_TO_FACTORY;
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&device->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&controller->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);
}
