#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <lwip/opt.h>

#include <spn/iface.h>
#include <spn/errno.h>

struct spn_cfg {
    bool dual_port; /* true: using two port and internal switch,
                       false: using single port and no switch */
};

struct spn_ctx {
    struct netif *iface_port1;
    struct netif *iface_port2;

    const struct spn_cfg *cfg;
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPN initialization
 *
 * @param ctx working context
 * @param cfg configurations
 * @param iface1 interface 1
 * @param iface2 interface 2 (optional port)
 * @return
 *          \c SPN_OK on success
 *          \c SPN_EINVAL if \c cfg is invalid
 */
int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg, iface_t* iface1, iface_t* iface2);

#if 0
/**
 * @brief input hook of ethernet frame
 *
 * @param frame
 * @param iface
 * @note in portiing context, lwip treats Werror=redundant-decls as error
 * @return \c  0 if frame is accepted
 *         \c -1 if frame is reject
 */
int spn_input_hook(void* frame, void* iface); 
#endif

#ifdef __cplusplus
}
#endif
