// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "platform/driver_instances.h"

static void i2c_slave_init(void)
{
#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
    rtos_i2c_slave_init(i2c_slave_ctx,
                        ~(1 << 0),
                        PORT_EXPANSION_1, //SCL
                        PORT_EXPANSION_3, //SDA
                        0x42);
#endif
}

static void gpio_init(void)
{
    static rtos_driver_rpc_t gpio_rpc_config;
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

#if ON_TILE(FLASH_TILE_NO)
    rtos_gpio_init(
            gpio_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx,
            &gpio_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_gpio_rpc_client_init(
            gpio_ctx,
            &gpio_rpc_config,
            intertile_ctx);
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    gpio_init();
    i2c_slave_init();
}
