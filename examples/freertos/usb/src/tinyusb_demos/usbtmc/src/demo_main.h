// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DEMO_MAIN_H_
#define DEMO_MAIN_H_

#include "rtos/drivers/gpio/api/rtos_gpio.h"

void create_tinyusb_demo(rtos_gpio_t *ctx, unsigned priority);
void led_indicator_pulse(void);

#endif /* DEMO_MAIN_H_ */
