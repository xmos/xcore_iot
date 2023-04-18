// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "platform_conf.h"
#include "platform/driver_instances.h"

static void gpio_start(void)
{
    rtos_gpio_rpc_config(gpio_ctx_t0, appconfGPIO_T0_RPC_PORT, appconfGPIO_RPC_PRIORITY);
    rtos_gpio_rpc_config(gpio_ctx_t1, appconfGPIO_T1_RPC_PORT, appconfGPIO_RPC_PRIORITY);

#if ON_TILE(0)
    rtos_gpio_start(gpio_ctx_t0);
#endif
#if ON_TILE(1)
    rtos_gpio_start(gpio_ctx_t1);
#endif
}

static void spi_start(void)
{
#if ON_TILE(0)
    rtos_spi_master_start(spi_master_ctx, appconfSPI_MASTER_TASK_PRIORITY);
#endif
}

static void i2c_start(void)
{
    rtos_i2c_master_rpc_config(i2c_master_ctx, appconfI2C_MASTER_RPC_PORT, appconfI2C_MASTER_RPC_PRIORITY);
#if ON_TILE(I2C_TILE_NO)
    rtos_i2c_master_start(i2c_master_ctx);
#endif
}

static void flash_start(void)
{
#if ON_TILE(0)
    uint32_t flash_core_map = ~((1 << appconfUSB_INTERRUPT_CORE) | (1 << appconfUSB_SOF_INTERRUPT_CORE));
    rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
    rtos_qspi_flash_op_core_affinity_set(qspi_flash_ctx, flash_core_map);
#endif
}

static void usb_start(void)
{
#if ON_TILE(USB_TILE_NO)
    usb_manager_start(appconfUSB_MANAGER_TASK_PRIORITY);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);

    gpio_start();
    spi_start();
    flash_start();
    i2c_start();
    usb_start();
}
