// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef GPI_CTRL_H_
#define GPI_CTRL_H_

#include <platform.h>
#include "rtos_gpio.h"

#if XK_VOICE_L71
#define BUTTON_MUTE_BITMASK 0x10
#define BUTTON_BTN_BITMASK  0x20
#define BUTTON_IP_2_BITMASK 0x40
#define BUTTON_IP_3_BITMASK 0x80
#define GPIO_BITMASK        (BUTTON_MUTE_BITMASK | BUTTON_BTN_BITMASK | BUTTON_IP_2_BITMASK | BUTTON_IP_3_BITMASK)
#define GPIO_PORT           PORT_GPI

#elif XCOREAI_EXPLORER
#define BUTTON_0_BITMASK    0x01
#define BUTTON_1_BITMASK    0x02
#define GPIO_BITMASK        (BUTTON_0_BITMASK | BUTTON_1_BITMASK)
#define GPIO_PORT           PORT_BUTTONS

#else
#define GPIO_PORT       0
#endif

void gpio_gpi_toggled_cb(uint32_t gpio_val);

void gpio_gpi_init(rtos_gpio_t *gpio_ctx);

#endif /* GPI_CTRL_H_ */
