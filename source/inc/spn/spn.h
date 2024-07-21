#pragma once

#include <spn/config.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/iface.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct spn_cfg {
  bool reserved;
};

struct spn_ctx {
  struct dcp_ctx dcp;
  struct db_ctx db;

  spn_iface_t ifaces[SPN_CONF_MAX_INTERFACE][SPN_CONF_MAX_PORT_PER_INTERFACE];
  const struct spn_cfg* cfg;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPN initialization
 *
 * @param ctx working context
 * @param cfg configurations
 *
 * @note This function not handle fatal error, it might cause memleak if failed to init
 * @return
 *          \c SPN_OK on success
 *          \c SPN_EINVAL if \c cfg is invalid
 */
int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg);

void spn_deinit(struct spn_ctx* ctx);

/**
 * @brief Initialize port
 *
 * @param ctx spn working context, usually don't need to be used
 * @param iface port interface
 * @param interface interface id
 * @param port port id
 *
 * @note This function is called by \c spn_init
 * @return
 *          \c SPN_OK on success
 *          \c SPN_EINVAL if \c iface is invalid
 *          \c SPN_ENOENT if \c interface is not found
 */
int spn_port_init(struct spn_ctx* ctx, struct spn_iface* iface, uint16_t interface, uint16_t port);

/**
 * @brief input hook of ethernet frame
 *
 * @param frame
 * @param iface
 * @return \c  0 if frame is accepted
 *         \c -1 if frame is reject
 */
int spn_input_hook(void* frame, void* iface);

/**
 * @brief Input hook of ethernet frame
 * @note this is a weak function that only hooked for test purpose
 */
void _spn_input_indication(int result);

#ifdef __cplusplus
}
#endif
