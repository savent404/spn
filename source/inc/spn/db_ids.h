#pragma once

typedef enum db_id {
    DB_ID_INVALID = 0,
    DB_ID_NAME_OF_STATION,
    DB_ID_NAME_OF_INTERFACE,
    DB_ID_NAME_OF_PORT,
    DB_ID_IP_MAC_ADDR,
    DB_ID_IP_ADDR,
    DB_ID_IP_MASK,
    DB_ID_IP_GATEWAY,
    DB_ID_IP_DNS,
    DB_ID_VENDOR_ID,
    DB_ID_DEVICE_ID,
    DB_ID_OEM_VENDOR_ID,
    DB_ID_OEM_DEVICE_ID
} db_id_t;
