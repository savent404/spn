#pragma once

#include <stddef.h>
#include <stdint.h>

#include <spn/errno.h>

struct spn_ctx {
    int _reserved;
};

struct spn_cfg {
    int _reserved;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPN initialization
 *
 * @param ctx working context
 * @param cfg configurations
 * @return
 *          \c SPN_OK on success
 *          \c SPN_EINVAL if \c cfg is invalid
 */
int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg);

/**
 * @brief input hook of ethernet frame
 *
 * @param frame
 * @param iface
 * @return \c  0 if frame is accepted
 *         \c -1 if frame is reject
 */
int spn_input_hook(void* frame, void* iface);

#ifdef __cplusplus
}
#endif
