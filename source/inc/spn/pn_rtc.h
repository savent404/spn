#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief
 *
 * \param rtc_frame
 * \param len
 * \return \c SPN_OK on success
 *         \c SPN_EAGAIN if the frame is not for us
 */
int spn_pn_rtc_input(void* rtc_frame, size_t len);

/**
 * \brief
 *
 * \param payload
 * \param len
 * \return \c SPN_OK on success
 *         \c SPN_EIO if ethernet frame transfer failed
 */
int spn_pn_rtc_output(void* payload, size_t len);

#ifdef __cplusplus
}
#endif
