// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>
#include <xs1.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

#include "rtos_printf.h"
#include <xcore/channel.h>

#include "example_pipeline/example_pipeline.h"

#define INTERTILE_TEST 0
#define RPC_TEST 0
#define GPIO_TEST 1
#define WIFI_TEST 1

#define I2C_TILE 1
#define PIPELINE_TILE 0
#define GPIO_TILE 0
#define SPI_TILE 1
#define QSPI_FLASH_TILE 1

#if RPC_TEST
#include "rpc_test/rpc_test.h"
#endif

#if INTERTILE_TEST
#include "intertile_stress_test/intertile_stress_test.h"
#endif

#if GPIO_TEST
#include "gpio_test/gpio_test.h"
#endif

#if WIFI_TEST
#include "wifi_test/wifi_test.h"
#endif

#include "board_init.h"

static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2s_master_t i2s_master_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;
static rtos_intertile_t intertile2_ctx_s;

static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2s_master_t *i2s_master_ctx = &i2s_master_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_intertile_t *intertile2_ctx = &intertile2_ctx_s;

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void loopback(void *arg)
{
    int32_t sample_buf[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS];

    for (;;) {
        rtos_mic_array_rx(
                mic_array_ctx,
                sample_buf,
                MIC_DUAL_FRAME_SIZE,
                portMAX_DELAY);

        for (int i = 0; i < MIC_DUAL_FRAME_SIZE; i++) {
            for (int j = 0; j < MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS; j++) {
                sample_buf[i][j] *= 40;
            }
        }

        rtos_i2s_master_tx(
                i2s_master_ctx,
                &sample_buf[0][0],
                MIC_DUAL_FRAME_SIZE);
    }
}


void vApplicationCoreInitHook(BaseType_t xCoreID)
{
#if ON_TILE(0)
    rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif

#if ON_TILE(1)
    rtos_printf("Initializing tile 1 core %d on core %d\n", xCoreID, portGET_CORE_ID());

    switch (xCoreID) {

    case 0:
        rtos_mic_array_interrupt_init(mic_array_ctx);
    }

#endif
}

#define I2C_MASTER_RPC_PORT 9
#define I2C_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define MIC_ARRAY_RPC_PORT 10
#define MIC_ARRAY_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define I2S_MASTER_RPC_PORT 11
#define I2S_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define GPIO_RPC_PORT 12
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define SPI_RPC_PORT 13
#define SPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define QSPI_RPC_PORT 14
#define QSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2 - 1)

void vApplicationDaemonTaskStartup(void *arg)
{
    uint32_t dac_configured;

    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    rtos_intertile_start(
            intertile_ctx);
    rtos_intertile_start(
            intertile2_ctx);

#if I2C_RPC_ENABLED
    rtos_i2c_master_rpc_config(i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);
#endif
#if MIC_ARRAY_RPC_ENABLED
    rtos_mic_array_rpc_config(mic_array_ctx, MIC_ARRAY_RPC_PORT, MIC_ARRAY_RPC_HOST_TASK_PRIORITY);
#endif
#if I2S_RPC_ENABLED
    rtos_i2s_master_rpc_config(i2s_master_ctx, I2S_MASTER_RPC_PORT, I2S_MASTER_RPC_HOST_TASK_PRIORITY);
#endif
#if GPIO_RPC_ENABLED
    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);
#endif
#if SPI_RPC_ENABLED
    rtos_spi_master_rpc_config(spi_master_ctx, SPI_RPC_PORT, SPI_RPC_HOST_TASK_PRIORITY);
#endif
#if QSPI_FLASH_RPC_ENABLED
    rtos_qspi_flash_rpc_config(qspi_flash_ctx, QSPI_RPC_PORT, QSPI_RPC_HOST_TASK_PRIORITY);
#endif

    #if ON_TILE(0)
    {
        rtos_printf("Starting GPIO driver\n");
        rtos_gpio_start(gpio_ctx);

        rtos_printf("Starting SPI driver\n");
        rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES-1);

        rtos_printf("Starting QSPI flash driver\n");
        rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES-1);

        rtos_printf("Starting i2c driver\n");
        rtos_i2c_master_start(i2c_master_ctx);
    }
    #endif

    #if ON_TILE(I2C_TILE)
    {
        int dac_init(rtos_i2c_master_t *i2c_ctx);

        if (dac_init(i2c_master_ctx) == 0) {
            rtos_printf("DAC initialization succeeded\n");
            dac_configured = 1;
        } else {
            rtos_printf("DAC initialization failed\n");
            dac_configured = 0;
        }
        chan_out_byte(other_tile_c, dac_configured);
    }
    #else
    {
        dac_configured = chan_in_byte(other_tile_c);
    }
    #endif

    chanend_free(other_tile_c);

    if (!dac_configured) {
        vTaskDelete(NULL);
    }

    #if ON_TILE(1)
    {
        /*
         * FIXME: It is necessary that these two tasks run on cores that are uninterrupted.
         * Therefore they must not run on core 0. It is currently difficult to guarantee this
         * as there is no support for core affinity.
         *
         * Setting I2C_TILE to 0 so that this task does not block above and remains on core 0
         * here will, however, ensure that these two tasks run on the two highest cores.
         */
        const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
                PDM_CLOCK_FREQUENCY,
                EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE);

        rtos_printf("Starting mic array driver\n");
        rtos_mic_array_start(
                mic_array_ctx,
                pdm_decimation_factor,
                rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
                rtos_mic_array_fir_compensation(pdm_decimation_factor),
                1.2 * MIC_DUAL_FRAME_SIZE,
                configMAX_PRIORITIES-1);

        rtos_printf("Starting i2s driver\n");
        rtos_i2s_master_start(
                i2s_master_ctx,
                rtos_i2s_master_mclk_bclk_ratio(AUDIO_CLOCK_FREQUENCY, EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE),
                I2S_MODE_I2S,
                1.2 * MIC_DUAL_FRAME_SIZE,
                configMAX_PRIORITIES-1);
    }
    #endif

    #if ON_TILE(PIPELINE_TILE)
    {
        example_pipeline_init(mic_array_ctx, i2s_master_ctx);
    }
    #endif

    #if RPC_TEST
    {
        rpc_test_init(intertile_ctx);
    }
    #endif

    #if INTERTILE_TEST
    {
        intertile_stress_test_start(intertile_ctx, intertile2_ctx);
    }
    #endif

    #if GPIO_TEST && ON_TILE(GPIO_TILE)
    {
        gpio_test_start(gpio_ctx);
    }
    #endif

    #if WIFI_TEST && ON_TILE(SPI_TILE)
    {
        wifi_test_start(wifi_device_ctx, gpio_ctx);
    }
    #endif

    #if ON_TILE(QSPI_FLASH_TILE)
    {
        uint8_t data[256];
        const int len = strlen("hello, world\n")+1;
        int erase = 0;

        rtos_qspi_flash_read(qspi_flash_ctx, data, 0, len);
        if (data[0] != 0xFF) {
            rtos_printf("First read: %s", data);
            erase = 1;
        } else {
            rtos_printf("First read appears empty\n");
        }

        rtos_qspi_flash_lock(qspi_flash_ctx);
        rtos_qspi_flash_erase(qspi_flash_ctx, 0, len);
        rtos_qspi_flash_write(qspi_flash_ctx, "hello, world\n", 0, len);
        rtos_qspi_flash_read(qspi_flash_ctx, data, 0, len);
        rtos_printf("Second read: %s", data);

        rtos_qspi_flash_read(qspi_flash_ctx, data, 0x123456, 256);
        for (int i = 0; i < 256; i++) {
            rtos_printf("%02x ", data[i]);
        }
        rtos_printf("\n");

        if (erase) {
            rtos_printf("Starting chip erase\n");
            rtos_qspi_flash_erase(qspi_flash_ctx, 0, rtos_qspi_flash_size_get(qspi_flash_ctx));
        }
        rtos_qspi_flash_unlock(qspi_flash_ctx);
    }
    #endif

    vTaskDelete(NULL);
}


void main_tile0(chanend_t c)
{
    board_tile0_init(c, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);

    other_tile_c = c;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                1,
                NULL);

    rtos_printf("start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

void main_tile1(chanend_t c)
{
    board_tile1_init(c, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);

    other_tile_c = c;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                1,
                NULL);

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
