// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "platform/driver_instances.h"

static rtos_gpio_t gpio_ctx_s;
rtos_gpio_t *gpio_ctx = &gpio_ctx_s;

static rtos_i2c_slave_t i2c_slave_ctx_s;
rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;

static rtos_intertile_t intertile_ctx_s;
rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
