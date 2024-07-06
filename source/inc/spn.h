#pragma once

#include <spn_iface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spn_instance {
    spn_iface_t ifaces[SPN_IFACE_MAX_NUM];
} spn_instance_t;

/**
 * \brief
 *
 * \param inst
 * \param mac
 * \return \c SPN_OK on success
 *         \c SPN_EEXIST if the instance already exists
 *
 */
int spn_init(spn_instance_t* inst, const uint8_t* mac, const uint8_t* ip);

/**
 * \brief
 *
 * \param inst
 * \return \c SPN_OK on success
 *         \c SPN_EINVAL if the instance does not exist or is not the same as the input
 */
int spn_deinit(spn_instance_t* inst);

spn_instance_t* spn_get_inst();

#ifdef __cplusplus
}
#endif
