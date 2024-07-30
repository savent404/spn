#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/spn.h>
#include "dcp_ctx.hpp"
#include "t_iface.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct Dcp : public ::testing::Test {
  dcp_inst_ptr controller = {};
  dcp_inst_ptr device = {};
  const uint8_t controller_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
  const uint8_t device_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
  spn_iface_t controller_iface = {};
  spn_iface_t device_iface = {};
  const uint8_t mcs_ident_addr[6] = DCP_MCS_MAC_ADDR;

  void SetUp() override {
    controller = std::make_shared<dcp_instance>();
    device = std::make_shared<dcp_instance>();
    controller->db_setup();
    device->db_setup();
    device->db_info_setup(0, 0x0a000001, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, "device", "iod", 0x1234, 0x5678, 0x01,
                          &device_iface);
    controller->db_info_setup(0, 0x0a000002, {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, "controller", "ioc", 0x1234, 0x5678,
                              0x02, &controller_iface);
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

void pack_ethernet(void* payload, const uint8_t* dst, uint8_t* src, uint16_t ethernet_type) {
  memcpy(payload, dst, 6);
  memcpy((char*)payload + 6, src, 6);
  memcpy((char*)payload + 12, &ethernet_type, 2);
}
}  // namespace

TEST_F(Dcp, ident) {
  char buf[1500];
  char* pdu_buf = buf + 14;
  uint16_t buf_len;

  // generate DCP.ident.req
  tain_buffer(buf, sizeof(buf));
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, pdu_buf, &buf_len), SPN_OK);

  // device receive DCP.ident.req
  EXPECT_EQ(dcp_input(&device->dcp, NULL, (const struct eth_addr*)mcs_ident_addr,
                      (const struct eth_addr*)controller_mac, pdu_buf, buf_len),
            SPN_OK);

  // -> device send DCP.ident.rep to controller, controller accept it by ident.cnf
  auto frame = iface_instace::get_instance()->fifo.front();
  ASSERT_NE(frame, nullptr);
  iface_instace::get_instance()->fifo.pop_front();
  EXPECT_EQ(dcp_input(&controller->dcp, NULL, (const struct eth_addr*)frame->dst, (const struct eth_addr*)device_mac,
                      frame->buff, frame->len),
            SPN_OK);
  delete frame;

  // Check result
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
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_MAC_ADDR, {.mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}})); /* rsp don't have it */
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_ADDR, {.u32 = 0x0a000001}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_MASK, {.u32 = 0x00FFFFFF}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_GATEWAY, {.u32 = 0}));
  EXPECT_TRUE(fn_check_obj(DB_ID_VENDOR_ID, {.u16 = 0x5678}));
  EXPECT_TRUE(fn_check_obj(DB_ID_DEVICE_ID, {.u16 = 0x1234}));
  EXPECT_TRUE(fn_check_obj(DB_ID_DEVICE_ROLE, {.u8 = 0x01}));
  EXPECT_TRUE(fn_check_obj(DB_ID_IP_BLOCK_INFO, {.u16 = 0x0001}));
}

TEST_F(Dcp, set) {
  char buf[1500];
  char* pdu_buf = buf + 14;
  uint16_t buf_len;

  // Use Ident to setup the device
  //
  //

  // generate DCP.ident.req
  tain_buffer(buf, sizeof(buf));
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, pdu_buf, &buf_len), SPN_OK);

  // device receive DCP.ident.req
  EXPECT_EQ(dcp_input(&device->dcp, NULL, (const struct eth_addr*)mcs_ident_addr,
                      (const struct eth_addr*)controller_mac, pdu_buf, buf_len),
            SPN_OK);

  // -> device send DCP.ident.rsp to controller, controller accept it by ident.cnf
  auto frame = iface_instace::get_instance()->fifo.front();
  ASSERT_NE(frame, nullptr);
  iface_instace::get_instance()->fifo.pop_front();
  EXPECT_EQ(dcp_input(&controller->dcp, NULL, (const struct eth_addr*)frame->dst, (const struct eth_addr*)device_mac,
                      frame->buff, frame->len),
            SPN_OK);
  delete frame;

  // generate DCP.set.req
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "fooboo";
  controller->dcp.ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
  controller->dcp.ucs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_CTRL_STOP;
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, pdu_buf, &buf_len), SPN_OK);

  // devcie receive DCP.set.req
  ASSERT_EQ(dcp_input(&device->dcp, NULL, (const struct eth_addr*)device_mac, (const struct eth_addr*)controller_mac,
                      pdu_buf, buf_len),
            SPN_OK);

  // -> device send DCP.set.rsp to controller, controller accept it by set.cnf
  frame = iface_instace::get_instance()->fifo.front();
  ASSERT_NE(frame, nullptr);
  iface_instace::get_instance()->fifo.pop_front();
  ASSERT_EQ(dcp_input(&controller->dcp, NULL, (const struct eth_addr*)frame->dst, (const struct eth_addr*)device_mac,
                      frame->buff, frame->len),
            SPN_OK);
  delete frame;

  // Check result
  struct db_object* obj;

  // device side
  ASSERT_EQ(db_get_interface_object(&device->db, 0, DB_ID_NAME_OF_INTERFACE, &obj), SPN_OK);
  ASSERT_STREQ(obj->data.str, "fooboo");

  // controller side
  ASSERT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION], DCP_BLOCK_ERR_OK);
  ASSERT_EQ(db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj),
            SPN_OK);
  EXPECT_STREQ(obj->data.str, "fooboo");
}

TEST_F(Dcp, get) {
  char buf[1500];
  char* pdu_buf = buf + 14;
  uint16_t buf_len;
  struct db_object* obj;

  // Use Ident to setup the device
  //
  //

  // generate DCP.ident.req
  tain_buffer(buf, sizeof(buf));
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, pdu_buf, &buf_len), SPN_OK);

  // device receive DCP.ident.req
  EXPECT_EQ(dcp_input(&device->dcp, NULL, (const struct eth_addr*)mcs_ident_addr,
                      (const struct eth_addr*)controller_mac, pdu_buf, buf_len),
            SPN_OK);

  // -> device send DCP.ident.rsp to controller, controller accept it by ident.cnf
  auto frame = iface_instace::get_instance()->fifo.front();
  ASSERT_NE(frame, nullptr);
  iface_instace::get_instance()->fifo.pop_front();
  EXPECT_EQ(dcp_input(&controller->dcp, NULL, (const struct eth_addr*)frame->dst, (const struct eth_addr*)device_mac,
                      frame->buff, frame->len),
            SPN_OK);
  delete frame;

  // generate DCP.get.req
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR;
  controller->dcp.ucs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.ucs_ctx.ex_ifr = SPN_EXTERNAL_INTERFACE_BASE;
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_get_req(&controller->dcp, &controller->dcp.ucs_ctx, pdu_buf, &buf_len), SPN_OK);

  // populate the data
  controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION] = DCP_BLOCK_ERR_RESOURCE_ERR;
  controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR] = DCP_BLOCK_ERR_RESOURCE_ERR;
  controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_IP_PARAMETER] = DCP_BLOCK_ERR_RESOURCE_ERR;
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj);
  db_free_objstr(obj);
  db_dup_str2obj(obj, "xxxxxx", 6);
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_VENDOR, &obj);
  db_free_objstr(obj);
  db_dup_str2obj(obj, "xxxxxx", 6);
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MAC_ADDR, &obj);
  obj->data.u32 = 0;
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_ADDR, &obj);
  obj->data.u32 = 0;
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MASK, &obj);
  obj->data.u32 = 0;

  // device receive DCP.get.req
  EXPECT_EQ(dcp_input(&device->dcp, NULL, (const struct eth_addr*)device_mac, (const struct eth_addr*)controller_mac,
                      pdu_buf, buf_len),
            SPN_OK);

  // -> device send DCP.get.rsp to controller, controller accept it by get.cnf
  frame = iface_instace::get_instance()->fifo.front();
  ASSERT_NE(frame, nullptr);
  iface_instace::get_instance()->fifo.pop_front();
  EXPECT_EQ(dcp_input(&controller->dcp, NULL, (const struct eth_addr*)frame->dst, (const struct eth_addr*)device_mac,
                      frame->buff, frame->len),
            SPN_OK);
  delete frame;

  // Check result
  EXPECT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION], DCP_BLOCK_ERR_OK);
  EXPECT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR], DCP_BLOCK_ERR_OPTION_NOT_SUPPORTED); /* FIXME: not supported in get.rsp */
  EXPECT_EQ(controller->dcp.ucs_ctx.resp_errors[DCP_BIT_IDX_IP_PARAMETER], DCP_BLOCK_ERR_OK);
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_INTERFACE, &obj);
  EXPECT_EQ(db_object_len(obj), 6);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "device", 6), 0);
#if 0
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_NAME_OF_VENDOR, &obj);
  EXPECT_EQ(db_object_len(obj), 3);
  EXPECT_EQ(db_is_static_string_object(obj), true);
  EXPECT_EQ(strncmp(obj->data.str, "iod", 3), 0);
#endif
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MAC_ADDR, &obj);
  EXPECT_EQ(obj->data.u32, 0);
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_ADDR, &obj);
  EXPECT_EQ(obj->data.u32, 0x0a000001);
  db_get_interface_object(&controller->db, SPN_EXTERNAL_INTERFACE_BASE, DB_ID_IP_MASK, &obj);
  EXPECT_EQ(obj->data.u32, 0x00FFFFFF);
}
