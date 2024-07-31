#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @This function is used by DCP to flash the port LED.
 * @note This function is acting as the trigger, after few seconds the LED should stop flashing.
 */
void spn_port_led_flash(void);

#ifdef __cplusplus
}
#endif
