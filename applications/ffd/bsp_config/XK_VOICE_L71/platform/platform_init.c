// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>

/* App headers */
#include "platform_conf.h"
#include "platform/app_pll_ctrl.h"
#include "platform/driver_instances.h"
#include "platform/platform_init.h"
#include "adaptive_rate_adjust.h"
#include "usb_support.h"

static void mclk_init(chanend_t other_tile_c)
{
#if ON_TILE(1)
    app_pll_init();
#endif
#if appconfUSB_ENABLED
    adaptive_rate_adjust_init(other_tile_c, MCLK_CLKBLK);
#endif
}

static void flash_init(void)
{
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,

            /** Derive QSPI clock from the 600 MHz xcore clock **/
            qspi_io_source_clock_xcore,

            /** Full speed clock configuration **/
            5, // 600 MHz / (2*5) -> 60 MHz,
            1,
            qspi_io_sample_edge_rising,
            0,

            /** SPI read clock configuration **/
            12, // 600 MHz / (2*12) -> 25 MHz
            0,
            qspi_io_sample_edge_falling,
            0,

            qspi_flash_page_program_1_4_4);
#endif
}

static void gpio_init(void)
{
    static rtos_driver_rpc_t gpio_rpc_config_t0;
    static rtos_driver_rpc_t gpio_rpc_config_t1;
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

#if ON_TILE(0)
    rtos_gpio_init(gpio_ctx_t0);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            client_intertile_ctx,
            1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            intertile_ctx);
#endif

#if ON_TILE(1)
    rtos_gpio_init(gpio_ctx_t1);

    rtos_gpio_rpc_client_init(
            gpio_ctx_t0,
            &gpio_rpc_config_t0,
            intertile_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx_t1,
            &gpio_rpc_config_t1,
            client_intertile_ctx,
            1);
#endif
}

static void i2c_init(void)
{
    static rtos_driver_rpc_t i2c_rpc_config;

#if ON_TILE(I2C_TILE_NO)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            400);

    rtos_i2c_master_rpc_host_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_i2c_master_rpc_client_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            intertile_ctx);
#endif
}

static void mics_init(void)
{
#if ON_TILE(MICARRAY_TILE_NO)
    rtos_mic_array_init(
            mic_array_ctx,
            (1 << appconfPDM_MIC_IO_CORE),
            RTOS_MIC_ARRAY_CHANNEL_SAMPLE);
#endif
}

static void i2s_init(void)
{
#if appconfI2S_ENABLED && ON_TILE(I2S_TILE_NO)
    port_t p_i2s_dout[1] = {
            PORT_I2S_DAC_DATA
    };
    port_t p_i2s_din[1] = {
            PORT_I2S_ADC_DATA
    };

    rtos_i2s_master_init(
            i2s_ctx,
            (1 << appconfI2S_IO_CORE),
            p_i2s_dout,
            1,
            p_i2s_din,
            1,
            PORT_I2S_BCLK,
            PORT_I2S_LRCLK,
            PORT_MCLK,
            I2S_CLKBLK);
#endif
}

static void usb_init(void)
{
#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    mclk_init(other_tile_c);
    gpio_init();
    flash_init();
    i2c_init();
    mics_init();
    i2s_init();
    usb_init();
}
