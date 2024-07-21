#pragma once

#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include "test_data.hpp"

namespace test {

using DataParser = test_data::dcp::DataParser;

struct Ddcp : public ::testing::Test {
  struct dcp_ctx dcp;
  struct db_ctx db;

  explicit Ddcp() {
    memset(&dcp, 0, sizeof(dcp));
    memset(&db, 0, sizeof(db));
  }

  void SetUp() override {
    struct db_interface* iface;

    db_init(&db);
    dcp_init(&dcp, &db);

    db_add_interface(&db, 0);
    db_get_interface(&db, 0, &iface);
    db_add_port(iface, 0);
  }

  void TearDown() override {
    dcp_deinit(&dcp);
    db_deinit(&db);
  }

  void declare_block_info(enum ip_block_info info) {
    db_value_t value;
    struct db_interface* iface;
    value.u16 = info;
    db_get_interface(&db, 0, &iface);
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_BLOCK_INFO, 0, 0, 0, &value);
  }

  void declare_ip_param(uint32_t ip, uint32_t mask, uint32_t gw) {
    db_value_t value;
    struct db_interface* iface;
    db_get_interface(&db, 0, &iface);
    value.u32 = ip;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_ADDR, 0, 0, 0, &value);
    value.u32 = mask;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_MASK, 0, 0, 0, &value);
    value.u32 = gw;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_GATEWAY, 0, 0, 0, &value);
  }

  void declare_name_of_station(const std::string name) {
    db_value_t value;
    struct db_interface* iface;
    if (strlen(name.c_str()) > 8) {
      value.ptr = strdup(name.c_str());
      db_get_interface(&db, 0, &iface);
      db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_STATION, 1, 1, strlen(name.c_str()), &value);
    } else {
      strcpy(value.str, name.c_str());
      db_get_interface(&db, 0, &iface);
      db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_STATION, 0, 1, strlen(name.c_str()), &value);
    }
  }

  void declare_name_of_vendor(const std::string name) {
    db_value_t value;
    struct db_interface* iface;
    if (strlen(name.c_str()) > 8) {
      value.ptr = strdup(name.c_str());
      db_get_interface(&db, 0, &iface);
      db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_VENDOR, 1, 1, strlen(name.c_str()), &value);
    } else {
      strcpy(value.str, name.c_str());
      db_get_interface(&db, 0, &iface);
      db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_VENDOR, 0, 1, strlen(name.c_str()), &value);
    }
  }

  void declare_device_id(uint16_t vendor_id, uint16_t device_id) {
    db_value_t value;
    struct db_interface* iface;

    db_get_interface(&db, 0, &iface);
    value.u16 = vendor_id;
    db_add_object(&iface->objects, db_id_t::DB_ID_VENDOR_ID, 0, 0, 0, &value);
    value.u16 = device_id;
    db_add_object(&iface->objects, db_id_t::DB_ID_DEVICE_ID, 0, 0, 0, &value);
  }

  void declare_device_role(uint16_t role) {
    db_value_t value;
    struct db_interface* iface;
    db_get_interface(&db, 0, &iface);
    value.u16 = role;
    db_add_object(&iface->objects, db_id_t::DB_ID_DEVICE_ROLE, 0, 0, 0, &value);
  }
};

}  // namespace test