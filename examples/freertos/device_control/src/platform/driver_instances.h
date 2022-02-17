// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_gpio.h"
#include "rtos_i2c_slave.h"
#include "rtos_intertile.h"

extern rtos_gpio_t *gpio_ctx;
extern rtos_i2c_slave_t *i2c_slave_ctx;
extern rtos_intertile_t *intertile_ctx;

#endif /* DRIVER_INSTANCES_H_ */
