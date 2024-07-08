#pragma once

#include <lwip/opt.h>

/**
 * @brief Response DCP device options that are supported by the device.
 *
 */
#ifndef SPN_DCP_IDENT_RESP_DEVICE_OPTIONS
#define SPN_DCP_IDENT_RESP_DEVICE_OPTIONS 1
#endif

/**
 * @brief Response DCP device station of name
 *
 */
#ifndef SPN_DCP_IDENT_RESP_STATION_OF_NAME
#define SPN_DCP_IDENT_RESP_STATION_OF_NAME 1
#endif

/**
 * @brief SPN GLOBAL DEBUG OPTION
 * @details set as LWIP_DBG_ON to enable it
 */
#ifndef SPN_DEBUG
#define SPN_DEBUG 0
#endif

#if SPN_DEBUG == LWIP_DBG_ON

#ifndef SPN_DCP_DEBUG
#define SPN_DCP_DEBUG 0
#endif

#ifndef SPN_PDU_DEBUG
#define SPN_PDU_DEBUG 0
#endif

#else
#define SPN_DCP_DEBUG 0
#define SPN_PDU_DEBUG 0
#endif
