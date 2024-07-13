#pragma once

#include <spn/config.h>

#include <spn/dcp.h> /* TODO: Give a public header to include, instead of private header */
#include <spn/dcp.h>
#include <spn/iface.h>

#ifndef SPN_WEAK_FN
#define SPN_WEAK_FN __attribute__((weak))
#endif

#ifdef __cplusplus
extern "C" {
#endif

void spn_sys_init(void);

/**
 * @brief Get ip address in big-endian
 *
 * @param netif net interface
 * @return uint32_t ip address, zero if failed
 */
uint32_t spn_sys_get_ip_addr(iface_t* iface);

/**
 * @brief Get mac address in big-endian
 *
 * @param netif net interface
 * @param[out] mac 6-byte mac address
 */
void spn_sys_get_mac_addr(iface_t* iface, char* mac);

/**
 * @brief Get ip mask in big-endian
 *
 * @param netif net interface
 * @return uint32_t mask 4-byte ip mask, zero if failed
 */
uint32_t spn_sys_get_ip_mask(iface_t* iface);

/**
 * @brief Get ip gateway in big-endian
 *
 * @param netif net interface
 * @return uint32_t gateway 4-byte ip gateway, zero if failed
 */
uint32_t spn_sys_get_ip_gw(iface_t* iface);

/**
 * @brief Get DNS ip big-endian
 *
 * @param netif net interface
 * @param idx DNS index (0~4)
 * @param return uint32_t DNS ip, zero if failed
 */
uint32_t spn_sys_get_dns(iface_t* iface, int idx);

/**
 * @brief Get name_of_station
 *
 * @return const char* station name, NULL if failed
 */
const char* spn_sys_get_station_name(void);

/**
 * @brief Get alias name
 *
 * @return const char* alias name, NULL if failed
 */
const char* spn_sys_get_alias_name(void);

/**
 * @brief Get vendor name
 *
 * @return const char* vendor name, NULL if failed
 */
const char* spn_sys_get_vendor_name(void);

/**
 * @brief Get device's vendor id
 *
 * @return uint16_t vendor id
 */
uint16_t spn_sys_get_vendor_id(void);

/**
 * @brief Get device's device id
 *
 * @return uint16_t device_id
 */
uint16_t spn_sys_get_device_id(void);

/**
 * @brief Get device's oem vendor id
 *
 * @return uint16_t oem vendor id
 */
uint16_t spn_sys_get_oem_vendor_id(void);

/**
 * @brief Get device's oem device id
 *
 * @return uint16_t oem vendor id
 */
uint16_t spn_sys_get_oem_device_id(void);

/**
 * @brief Get device's role
 *
 * @return \c spn_role role id
 */
enum spn_role spn_sys_get_role(void);

/**
 * @brief Get device's ip status
 *
 * @return \c spn_ip_status ip status
 */
enum spn_ip_status spn_sys_get_ip_status(void);

/**
 * @brief get device's instance
 * 
 * @return uint16_t big-endian instance
 */
uint16_t spn_sys_get_dev_instance(void);


/**
 * @brief dev initiative
 * 
 * @return uint16_t big-endian initiative
 */
uint16_t spn_sys_get_dev_initiative(void);

/**
 * @brief Get device's netif
 *
 * @param interface instance of pn
 * @param port      port of instance
 * @return struct netif*
 */
struct netif* spn_sys_get_netif(int interface, int port);

#ifdef __cplusplus
}
#endif
