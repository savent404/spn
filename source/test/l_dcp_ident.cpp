#include <gtest/gtest.h>
#include <spn/db_ll.h>
#include <memory>
#include "dcp_ctx.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct DcpIdent : public ::testing::Test {
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

TEST_F(DcpIdent, ident_all) {
  static char buf[1500];
  uint16_t buf_len;
  uint16_t exiface;

  // ident all
  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, &exiface), SPN_OK);
  EXPECT_EQ(exiface, SPN_EXTERNAL_INTERFACE_BASE);
}

TEST_F(DcpIdent, ident_all_infomation) {
  static char buf[1500];
  uint16_t buf_len;

  // ident all
  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  EXPECT_EQ(controller->dcp.mcs_ctx.state, DCP_STATE_IDENT_REQ);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
  EXPECT_EQ(controller->dcp.mcs_ctx.external_interface_id, SPN_EXTERNAL_INTERFACE_BASE + 1);

  struct db_interface* interface;
  ASSERT_EQ(db_get_interface(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, &interface), SPN_OK);

  auto fn_check_obj = [&](db_id_t id, db_value_t val) -> bool {
    struct db_object* obj;
    int res = db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, id, &obj);
    if (res != SPN_OK) {
      return false;
    }

    if (!obj->attr.is_dyn) {
      return !memcmp(&obj->data, &val, db_object_len(obj));
    } else {
      return !memcmp(obj->data.ptr, &val, db_object_len(obj));
    }
    return false;
  };

  EXPECT_TRUE(fn_check_obj(DB_ID_NAME_OF_INTERFACE, {.str = "device"}));
  EXPECT_TRUE(fn_check_obj(DB_ID_NAME_OF_VENDOR, {.str = "iod"}));
  EXPECT_FALSE(fn_check_obj(DB_ID_IP_MAC_ADDR, {.mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}})); /* rsp don't have it */
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_ADDR, {.u32 = 0x0a000001}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_MASK, {.u32 = 0x00FFFFFF}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_GATEWAY, {.u32 = 0}));
  EXPECT_TRUE(fn_check_obj(DB_ID_VENDOR_ID, {.u16 = 0x5678}));
  EXPECT_TRUE(fn_check_obj(DB_ID_DEVICE_ID, {.u16 = 0x1234}));
  EXPECT_TRUE(fn_check_obj(DB_ID_DEVICE_ROLE, {.u8 = 0x01}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_BLOCK_INFO, {.u16 = 0x0001}));
}

TEST_F(DcpIdent, ident_station_name) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpIdent, ident_alias_name) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-001.device";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpIdent, ident_multiple) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-001.device";

  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.mcs_ctx.ip_addr = 0x0a000001;
  controller->dcp.mcs_ctx.ip_mask = 0x00FFFFFF;
  controller->dcp.mcs_ctx.ip_gw = 0;

  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_DEVICE_ID;
  controller->dcp.mcs_ctx.device_id = 0x1234;
  controller->dcp.mcs_ctx.vendor_id = 0x5678;

  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR;
  controller->dcp.mcs_ctx.vendor_name = "iod";

  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpIdent, ident_req_failpath) {
  static char buf[1500];
  struct dcp_mcr_ctx mcr;
  uint16_t buf_len;

  auto fn_clean_req = [&]() {
    tain_buffer(buf, 1500);
    controller->dcp.mcs_ctx.req_options_bitmap = 0;
    controller->dcp.mcs_ctx.state = DCP_STATE_IDLE;
    controller->dcp.mcs_ctx.xid++;
    memset(&mcr, 0, sizeof(mcr));
  };

  // due to busy
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), -SPN_EBUSY);
}

TEST_F(DcpIdent, ident_ind_failpath) {
  static char buf[1500];
  struct dcp_mcr_ctx mcr;
  uint16_t buf_len;

  auto fn_clean_req = [&]() {
    tain_buffer(buf, 1500);
    controller->dcp.mcs_ctx.req_options_bitmap = 0;
    controller->dcp.mcs_ctx.state = DCP_STATE_IDLE;
    controller->dcp.mcs_ctx.xid++;
    memset(&mcr, 0, sizeof(mcr));
  };

  // pass
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  // due to payload length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, 13), -SPN_EBADMSG);

  // due to option is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  buf[12] = 0x00;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len), -SPN_EAGAIN);

  // due to option is empty
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  buf[11] = 0x00;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to station name is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "devicefooooo";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to alias name is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-001.devicE";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);
  // due to alias name is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-002.device";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to vendor name is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR;
  controller->dcp.mcs_ctx.vendor_name = "iodfooo";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to ip parameter(gw) is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.mcs_ctx.ip_addr = 0x0a000001;
  controller->dcp.mcs_ctx.ip_mask = 0x00FFFFFF;
  controller->dcp.mcs_ctx.ip_gw = 0x0a000002;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to ip parameter(mask) is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.mcs_ctx.ip_addr = 0x0a000001;
  controller->dcp.mcs_ctx.ip_mask = 0x00FFFFFE;
  controller->dcp.mcs_ctx.ip_gw = 0;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to ip parameter(addr) is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.mcs_ctx.ip_addr = 0x0a000002;
  controller->dcp.mcs_ctx.ip_mask = 0x00FFFFFF;
  controller->dcp.mcs_ctx.ip_gw = 0;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to device id
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_DEVICE_ID;
  controller->dcp.mcs_ctx.device_id = 0x1235;
  controller->dcp.mcs_ctx.vendor_id = 0x5678;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);

  // due to vendor id is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_DEVICE_ID;
  controller->dcp.mcs_ctx.device_id = 0x1234;
  controller->dcp.mcs_ctx.vendor_id = 0x5679;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), -SPN_EAGAIN);
}

TEST_F(DcpIdent, ident_cnf_failpath) {
  static char buf[1500];
  struct dcp_mcr_ctx mcr;
  uint16_t buf_len;

  auto fn_clean_req = [&]() {
    tain_buffer(buf, 1500);
    controller->dcp.mcs_ctx.req_options_bitmap = 0;
    controller->dcp.mcs_ctx.state = DCP_STATE_IDLE;
    controller->dcp.mcs_ctx.xid++;
    memset(&mcr, 0, sizeof(mcr));
  };

  // due to payload length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, 13, 0), -SPN_EBADMSG);

  // due to string block length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[2 + 10 + 4 + 14 + 2] = 0xFF;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), -SPN_EBADMSG);

  // due to ip block length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[2 + 10 + 2] = 0xFF;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), -SPN_EBADMSG);

  // due to device id block length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[0x37] = 0xFF;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), -SPN_EBADMSG);

  // due to role block length is not match
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[0x41] = 0xFF;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), -SPN_EBADMSG);

  // due to options are empty
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[0x49] = 0x00;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), -SPN_EBADMSG);

  // due to unknow options
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  buf[0x1F] = 0xff;
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len, 0), SPN_OK);

  // due to xid is not match  fn_clean_req();
  fn_clean_req();
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);
  mcr.xid++;
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), -SPN_ENXIO);
}

TEST_F(DcpIdent, unspported_filter) {}
