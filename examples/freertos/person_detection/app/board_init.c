// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "board_init.h"
#include "app_conf.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile1_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t* ov_device_ctx,
        rtos_gpio_t *gpio_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile1);

    rtos_gpio_init(
            gpio_ctx);

    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);

    rtos_spi_master_init(
            spi_master_ctx,
            XS1_CLKBLK_1,
            XS1_PORT_1A,
            XS1_PORT_1L,
            XS1_PORT_1J,
            XS1_PORT_1M);

    /* Camera supports 8 MHz max */
    rtos_spi_master_device_init(
            ov_device_ctx,
            spi_master_ctx,
            0,
            SPI_MODE_0,
            spi_master_source_clock_ref,
            8, /* 100 MHz / 4 * div = 3.125 MHz  */
            spi_master_sample_delay_2, /* what should this be? 2? 3? 4? */
            0, /* should this be > 0 if the above is 3-4 ? */
            1,
            0,
            0);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile0);
}
