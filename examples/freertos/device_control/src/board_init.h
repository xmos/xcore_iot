// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BOARD_INIT_H_
#define BOARD_INIT_H_

#include "rtos/drivers/i2c/api/rtos_i2c_slave.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_slave_t *i2c_slave_ctx,
        rtos_gpio_t *gpio_ctx);

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_gpio_t *gpio_ctx);

#endif /* BOARD_INIT_H_ */
