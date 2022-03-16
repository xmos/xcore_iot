// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "app_pll_ctrl.h"
#include "adaptive_rate_adjust.h"
#include "usb_support.h"
#include "driver_instances.h"

/** TILE 0 Clock Blocks */
#define FLASH_CLKBLK  XS1_CLKBLK_1
#if XCOREAI_EXPLORER
#define MCLK_CLKBLK   XS1_CLKBLK_2
#endif
#define SPI_CLKBLK    XS1_CLKBLK_3
#define XUD_CLKBLK_1  XS1_CLKBLK_4 /* Reserved for lib_xud */
#define XUD_CLKBLK_2  XS1_CLKBLK_5 /* Reserved for lib_xud */

/** TILE 1 Clock Blocks */
#define PDM_CLKBLK_1  XS1_CLKBLK_1
#define PDM_CLKBLK_2  XS1_CLKBLK_2
#define I2S_CLKBLK    XS1_CLKBLK_3
#if XVF3610_Q60A || OSPREY_BOARD
#define MCLK_CLKBLK   XS1_CLKBLK_4
#endif

#if XVF3610_Q60A
#define PORT_MCLK           PORT_MCLK_IN_OUT
#define PORT_SQI_CS         PORT_SQI_CS_0
#define PORT_SQI_SCLK       PORT_SQI_SCLK_0
#define PORT_SQI_SIO        PORT_SQI_SIO_0
#define PORT_I2S_DAC_DATA   I2S_DATA_IN
#define PORT_I2S_ADC_DATA   I2S_MIC_DATA
#define PORT_I2C_SLAVE_SCL  PORT_I2C_SCL
#define PORT_I2C_SLAVE_SDA  PORT_I2C_SDA
#elif XCOREAI_EXPLORER
#define PORT_MCLK           PORT_MCLK_IN
#elif OSPREY_BOARD
#define PORT_MCLK           PORT_MCLK_IN
#else
#error Unsupported board
#endif

static void mclk_init(chanend_t other_tile_c)
{
#if !appconfEXTERNAL_MCLK && ON_TILE(1)
    app_pll_init();
#endif
#if appconfUSB_ENABLED
    adaptive_rate_adjust_init(other_tile_c, MCLK_CLKBLK);
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

static void i2c_init(void)
{
#if ON_TILE(I2C_TILE_NO)
#if XCOREAI_EXPLORER || (XVF3610_Q60A && !appconfI2C_CTRL_ENABLED)
    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);
#endif
#endif

#if appconfI2C_CTRL_ENABLED && ON_TILE(I2C_CTRL_TILE_NO)
    rtos_i2c_slave_init(i2c_slave_ctx,
                        (1 << appconfI2C_IO_CORE),
                        PORT_I2C_SLAVE_SCL,
                        PORT_I2C_SLAVE_SDA,
                        appconf_CONTROL_I2C_DEVICE_ADDR);
#endif
}

static void spi_init(void)
{
#if appconfSPI_OUTPUT_ENABLED && ON_TILE(SPI_OUTPUT_TILE_NO)
    rtos_spi_slave_init(spi_slave_ctx,
                        (1 << appconfSPI_IO_CORE),
                        SPI_CLKBLK,
                        SPI_MODE_3,
                        WIFI_CLK,
                        WIFI_MOSI,
                        WIFI_MISO,
                        XS1_PORT_1A);
#endif
}

static void mics_init(void)
{
#if ON_TILE(AUDIO_HW_TILE_NO)
    rtos_mic_array_init(
            mic_array_ctx,
            (1 << appconfPDM_MIC_IO_CORE),
            RTOS_MIC_ARRAY_SAMPLE_CHANNEL);
#endif
}

static void i2s_init(void)
{
#if appconfI2S_ENABLED && ON_TILE(AUDIO_HW_TILE_NO)
    port_t p_i2s_dout[1] = {
            PORT_I2S_DAC_DATA
    };
    port_t p_i2s_din[1] = {
            PORT_I2S_ADC_DATA
    };

#if appconfI2S_MODE == appconfI2S_MODE_MASTER
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
#elif appconfI2S_MODE == appconfI2S_MODE_SLAVE
    rtos_i2s_slave_init(
            i2s_ctx,
            (1 << appconfI2S_IO_CORE),
            p_i2s_dout,
            1,
            p_i2s_din,
            1,
            PORT_I2S_BCLK,
            PORT_I2S_LRCLK,
            I2S_CLKBLK);
#else
#error Invalid I2S mode
#endif
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
    spi_init();
    mics_init();
    i2s_init();
    usb_init();
}
