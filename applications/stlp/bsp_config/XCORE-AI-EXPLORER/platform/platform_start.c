// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "fs_support.h"

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"
#include "aic3204.h"
#include "usb_support.h"

#if appconfI2C_CTRL_ENABLED
#include "app_control/app_control.h"
#include "device_control_i2c.h"
#endif

extern void i2s_rate_conversion_enable(void);

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

static void flash_start(void)
{
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
#endif
}

static void i2c_master_start(void)
{
#if !appconfI2C_CTRL_ENABLED
    rtos_i2c_master_rpc_config(i2c_master_ctx, appconfI2C_MASTER_RPC_PORT, appconfI2C_MASTER_RPC_PRIORITY);

#if ON_TILE(I2C_TILE_NO)
    rtos_i2c_master_start(i2c_master_ctx);
#endif
#endif
}

static void audio_codec_start(void)
{
#if !appconfI2C_CTRL_ENABLED
#if appconfI2S_ENABLED
    int ret = 0;
#if ON_TILE(I2C_TILE_NO)
    if (aic3204_init() != 0) {
        rtos_printf("DAC initialization failed\n");
    }
    rtos_intertile_tx(intertile_ctx, 0, &ret, sizeof(ret));
#else
    rtos_intertile_rx_len(intertile_ctx, 0, RTOS_OSAL_WAIT_FOREVER);
    rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
#endif
#endif
#endif
}

static void i2c_slave_start(void)
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

static void spi_start(void)
{
#if appconfSPI_OUTPUT_ENABLED && ON_TILE(SPI_OUTPUT_TILE_NO)

    const rtos_gpio_port_id_t wifi_rst_port = rtos_gpio_port(WIFI_WUP_RST_N);
    rtos_gpio_port_enable(gpio_ctx_t0, wifi_rst_port);
    rtos_gpio_port_out(gpio_ctx_t0, wifi_rst_port, 0x00);

    const rtos_gpio_port_id_t wifi_cs_port = rtos_gpio_port(WIFI_CS_N);
    rtos_gpio_port_enable(gpio_ctx_t0, wifi_cs_port);
    rtos_gpio_port_out(gpio_ctx_t0, wifi_cs_port, 0x0F);

    rtos_spi_slave_start(spi_slave_ctx,
                         NULL,
                         (rtos_spi_slave_start_cb_t) spi_slave_start_cb,
                         (rtos_spi_slave_xfer_done_cb_t) spi_slave_xfer_done_cb,
                         appconfSPI_INTERRUPT_CORE,
                         appconfSPI_TASK_PRIORITY);
#endif
}

static void mics_start(void)
{
    rtos_mic_array_rpc_config(mic_array_ctx, appconfMIC_ARRAY_RPC_PORT, appconfMIC_ARRAY_RPC_PRIORITY);

#if ON_TILE(MICARRAY_TILE_NO)
    rtos_mic_array_start(
            mic_array_ctx,
            2 * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
            appconfPDM_MIC_INTERRUPT_CORE);
#endif
}

static void i2s_start(void)
{
#if appconfI2S_ENABLED
    rtos_i2s_rpc_config(i2s_ctx, appconfI2S_RPC_PORT, appconfI2S_RPC_PRIORITY); 

#if ON_TILE(I2S_TILE_NO)
    if (appconfI2S_AUDIO_SAMPLE_RATE == 3*appconfAUDIO_PIPELINE_SAMPLE_RATE) {
        i2s_rate_conversion_enable();
    }

    rtos_i2s_start(
            i2s_ctx,
            rtos_i2s_mclk_bclk_ratio(appconfAUDIO_CLOCK_FREQUENCY, appconfPIPELINE_AUDIO_SAMPLE_RATE),
            I2S_MODE_I2S,
            2.2 * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
            1.2 * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
            appconfI2S_INTERRUPT_CORE);
#endif
#endif
}

static void usb_start(void)
{
#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_start(appconfUSB_MGR_TASK_PRIORITY);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);

    gpio_start();
    flash_start();
    i2c_master_start();
    audio_codec_start();
    i2c_slave_start();
    spi_start();
    mics_start();
    i2s_start();
    usb_start();
}
