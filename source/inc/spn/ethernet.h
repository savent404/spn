#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Ethernet frame type
 *
 */
#define SPN_ETH_TYPE_IP 0x0800
#define SPN_ETH_TYPE_VLAN 0x8100
#define SPN_ETH_TYPE_PN 0x8892
/**
 */

#define SPN_ETH_SRC_MAC(f) ((char*)((f)->src_mac))
#define SPN_ETH_DST_MAC(f) ((char*)((f)->dst_mac))
#define SPN_ETH_TYPE(f) ((f)->ethertype)
#define SPN_ETH_PAYLOAD(f) ((void*)(&((f)->payload[0])))

/**
 * \brief Ethernet frame structure
 *
 */
#pragma pack(push, 1)
typedef struct spn_ethernet {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uintptr_t payload[1];
} spn_ethernet_t;
#pragma pack(pop)

struct spn_iface;
typedef struct spn_iface spn_iface_t;

/**
 * \brief handler dealing with incoming ethernet frames
 *
 * \param iface
 * \param ethernet_frame
 * \param len
 * \return \c SPN_EAGAIN - if the frame is not for us
 *         \c SPN_OK    - if the frame is for us
 *
 */
int spn_eth_input(spn_iface_t* iface, void* ethernet_frame, size_t len);

/**
 * \brief handler dealing with outgoing ethernet frames
 *
 * \param iface
 * \param ethernet_frame
 * \param len
 * \return \c SPN_OK    - if the frame is sent successfully
 *         \c SPN_EIO   - if the frame is not sent successfully
 */
int spn_eth_output(spn_iface_t* iface, const void* ethernet_frame, size_t len);

#ifdef __cplusplus
}
#endif
