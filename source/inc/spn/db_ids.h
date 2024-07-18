#pragma once

typedef enum db_id {
    DB_ID_INVALID = 0,
    DB_ID_NAME_OF_STATION,
    DB_ID_NAME_OF_VENDOR,
    DB_ID_NAME_OF_INTERFACE,
    DB_ID_NAME_OF_PORT,
    DB_ID_IP_MAC_ADDR,
    DB_ID_IP_ADDR,
    DB_ID_IP_MASK,
    DB_ID_IP_GATEWAY,
    DB_ID_IP_DNS,
    DB_ID_IP_BLOCK_INFO,
    DB_ID_VENDOR_ID,
    DB_ID_DEVICE_ID,
    DB_ID_DEVICE_ROLE,
    DB_ID_DEVICE_INSTANCE,
    DB_ID_DEVICE_OPTIONS, /* options that device supported */
    DB_ID_OEM_VENDOR_ID,
    DB_ID_OEM_DEVICE_ID
} db_id_t;

enum ip_block_info {
    IP_BLOCK_INFO_NONE = 0,
    IP_BLOCK_INFO_STATIC = 1,
    IP_BLOCK_INFO_DHCP = 2,
};

enum device_role {
    DEV_ROLE_DEVICE_BIT = 0,
    DEV_ROLE_CONTROLLER_BIT = 1,
    DEV_ROLE_SUPERVISOR_BIT = 2,
};
