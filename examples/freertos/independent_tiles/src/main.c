// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

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
#define QSPI_TEST 0
#define SW_MEM_TEST 1

#if WIFI_TEST && QSPI_TEST
#error Cannot test the QSPI when the WIFI test is enabled
#endif

#define I2C_TILE 1
#define PIPELINE_TILE 0
#define GPIO_TILE 0
#define WIFI_TILE 1 /* Uses SPI, GPIO, and QSPI */
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
#include "fs_support.h"
#endif

#if SW_MEM_TEST
#include "swmem_test/swmem_test.h"
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
        break;
    case 1:
        rtos_i2s_master_interrupt_init(i2s_master_ctx);
        break;
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

#if SW_MEM_TEST && ON_TILE(SWMEM_TILE)
    swmem_test_run();
#endif

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

    #if WIFI_TEST && ON_TILE(WIFI_TILE)
    {
        rtos_fatfs_init(qspi_flash_ctx);
        wifi_test_start(wifi_device_ctx, gpio_ctx);
    }
    #endif

    #if QSPI_TEST && ON_TILE(QSPI_FLASH_TILE)
    {
        const char test_str[] = "hello, world\n";
        const int len = strlen(test_str) + 1;
        uint8_t data[len];
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
        rtos_qspi_flash_write(qspi_flash_ctx, (const uint8_t *) test_str, 0, len);
        rtos_qspi_flash_read(qspi_flash_ctx, data, 0, len);
        rtos_printf("Second read: %s", data);

        if (erase) {
            rtos_printf("Starting chip erase\n");
            rtos_qspi_flash_erase(qspi_flash_ctx, 0, rtos_qspi_flash_size_get(qspi_flash_ctx));
        }
        rtos_qspi_flash_unlock(qspi_flash_ctx);
    }
    #endif

    vTaskDelete(NULL);
}


#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
#if SW_MEM_TEST && ON_TILE(SWMEM_TILE)
    swmem_test_init();
#endif

    (void) c0;
    board_tile0_init(c1, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

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
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
#if SW_MEM_TEST && ON_TILE(SWMEM_TILE)
    swmem_test_init();
#endif

    board_tile1_init(c0, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                configMAX_PRIORITIES-1,
                NULL);

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
#endif
