// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "board_init.h"

static rtos_driver_rpc_t gpio_rpc_config;

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_slave_t *i2c_slave_ctx,
        rtos_gpio_t *gpio_ctx)
{
    rtos_intertile_init(intertile_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_i2c_slave_init(i2c_slave_ctx,
                        ~(1 << 0),
                        PORT_EXPANSION_1, //SCL
                        PORT_EXPANSION_3, //SDA
                        0x42);

    rtos_gpio_init(
            gpio_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx,
            &gpio_rpc_config,
            client_intertile_ctx,
            1);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_gpio_t *gpio_ctx)
{
    rtos_intertile_init(intertile_ctx, tile0);

    rtos_gpio_rpc_client_init(
            gpio_ctx,
            &gpio_rpc_config,
            intertile_ctx);
}
