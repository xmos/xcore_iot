// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "device_control_i2c.h"

#include "app_conf.h"
#include "app_control/app_control.h"
#include "platform/driver_instances.h"

static void i2c_start(void)
{
#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
    rtos_i2c_slave_start(i2c_slave_ctx,
                         device_control_i2c_ctx,
                         (rtos_i2c_slave_start_cb_t) device_control_i2c_start_cb,
                         (rtos_i2c_slave_rx_cb_t) device_control_i2c_rx_cb,
                         (rtos_i2c_slave_tx_start_cb_t) device_control_i2c_tx_start_cb,
                         (rtos_i2c_slave_tx_done_cb_t) NULL,
                         appconfI2C_INTERRUPT_CORE,
                         appconfI2C_TASK_PRIORITY);
#endif
}

static void gpio_start(void)
{
    rtos_gpio_rpc_config(gpio_ctx, appconfGPIO_RPC_PORT, appconfGPIO_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(GPIO_TILE_NO)
    rtos_gpio_start(gpio_ctx);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);

    gpio_start();
    i2c_start();
}
