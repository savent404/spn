#pragma once

#include <assert.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/iface.h>
#include <spn/spn.h>
#include <memory>

namespace test {

struct dcp_instance {
  struct dcp_ctx dcp;
  struct db_ctx db;

  dcp_instance() {
    db_init(&db);
    dcp_init(&dcp, &db);
  }

  ~dcp_instance() {
    dcp_deinit(&dcp);
    db_deinit(&db);
  }

  void db_setup() {
    struct db_interface* interface;
    db_add_interface(&db, 0);
    db_get_interface(&db, 0, &interface);
    db_add_port(interface, 0);
  }

  void db_info_setup(int interface_id,
                     uint32_t ip,
                     const eth_addr& mac,
                     const char* station,
                     const char* vendor,
                     uint16_t device_id,
                     uint16_t vendor_id,
                     uint8_t role,
                     spn_iface_t* iface_1 = nullptr) {
    struct db_interface* interface;
    struct db_port* port;
    struct db_object* obj;
    db_value_t val;
    int res;

    res = db_get_interface(&db, interface_id, &interface);
    assert(res == SPN_OK);

    res = db_get_port(interface, 0, &port);
    assert(res == SPN_OK);

    memcpy(val.str, "port-001", 8);
    res = db_add_object(&port->objects, DB_ID_NAME_OF_PORT, 0, 1, 8, &val);
    assert(res == SPN_OK);

    if (iface_1) {
      val.ptr = iface_1;
      res = db_add_object(&port->objects, DB_ID_IFACE, 0, 0, sizeof(spn_iface_t*), &val);
      assert(res == SPN_OK);
    }

    memcpy(val.mac, mac.addr, 6);
    res = db_add_object(&interface->objects, DB_ID_IP_MAC_ADDR, 0, 0, 6, &val);
    assert(res == SPN_OK);

    val.u32 = ip;
    res = db_add_object(&interface->objects, DB_ID_IP_ADDR, 0, 0, 4, &val);
    assert(res == SPN_OK);

    val.u32 = 0x00FFFFFF;
    res = db_add_object(&interface->objects, DB_ID_IP_MASK, 0, 0, 4, &val);
    assert(res == SPN_OK);

    val.u32 = 0;
    res = db_add_object(&interface->objects, DB_ID_IP_GATEWAY, 0, 0, 4, &val);
    assert(res == SPN_OK);

    val.u16 = vendor_id;
    res = db_add_object(&interface->objects, DB_ID_VENDOR_ID, 0, 0, 2, &val);
    assert(res == SPN_OK);

    val.u16 = device_id;
    res = db_add_object(&interface->objects, DB_ID_DEVICE_ID, 0, 0, 2, &val);
    assert(res == SPN_OK);

    val.u8 = role;
    res = db_add_object(&interface->objects, DB_ID_DEVICE_ROLE, 0, 0, 1, &val);
    assert(res == SPN_OK);

    val.u16 = ip_block_info::IP_BLOCK_INFO_STATIC;
    res = db_add_object(&interface->objects, DB_ID_IP_BLOCK_INFO, 0, 0, 2, &val);
    assert(res == SPN_OK);

    if (strlen(station) > 8) {
      val.ptr = strdup(station);
      res = db_add_object(&interface->objects, DB_ID_NAME_OF_INTERFACE, 1, 1, strlen(station), &val);
      assert(res == SPN_OK);
    } else {
      memcpy(val.str, station, strlen(station));
      res = db_add_object(&interface->objects, DB_ID_NAME_OF_INTERFACE, 0, 1, strlen(station), &val);
      assert(res == SPN_OK);
    }

    if (strlen(vendor) > 8) {
      val.ptr = strdup(vendor);
      res = db_add_object(&interface->objects, DB_ID_NAME_OF_VENDOR, 1, 1, strlen(vendor), &val);
      assert(res == SPN_OK);
    } else {
      memcpy(val.str, vendor, strlen(vendor));
      res = db_add_object(&interface->objects, DB_ID_NAME_OF_VENDOR, 0, 1, strlen(vendor), &val);
      assert(res == SPN_OK);
    }
  }
};
using dcp_inst_ptr = std::shared_ptr<dcp_instance>;

}  // namespace test
