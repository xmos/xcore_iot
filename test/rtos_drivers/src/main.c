// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/i2c/api/rtos_i2c_slave.h"
#include "rtos/drivers/i2s/api/rtos_i2s.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/mic_array/api/rtos_mic_array.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "rtos_test/rtos_test_utils.h"
#include "individual_tests/individual_tests.h"

static rtos_intertile_t intertile_ctx_s;
static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_i2s_t i2s_ctx_s;
static rtos_i2c_slave_t i2c_slave_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_i2s_t *i2s_ctx = &i2s_ctx_s;
static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;

chanend_t other_tile_c;

void vApplicationMallocFailedHook( void )
{
    kernel_printf("Malloc Failed!");
    configASSERT(0);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char* pcTaskName )
{
    kernel_printf("Stack Overflow! %s", pcTaskName);
    configASSERT(0);
}

void vApplicationDaemonTaskStartup(void *arg)
{
    /* Intertile test must always before any test that uses RPC */
    if (RUN_INTERTILE_TESTS) {
        intertile_device_tests(intertile_ctx, other_tile_c);
    } else {
        test_printf("Skipped INTERTILE tests");
        test_printf("Starting intertile device");
        rtos_intertile_start(intertile_ctx);
    }

    if (RUN_GPIO_TESTS) {
        gpio_device_tests(gpio_ctx, other_tile_c);
    } else {
        test_printf("Skipped GPIO tests");
    }

    if (RUN_I2C_TESTS) {
        i2c_device_tests(i2c_master_ctx, i2c_slave_ctx, other_tile_c);
    } else {
        test_printf("Skipped I2C tests");
    }


    // rtos_gpio_rpc_config(gpio_ctx_t0, appconfGPIO_T0_RPC_PORT, appconfGPIO_RPC_HOST_PRIORITY);
    // rtos_gpio_rpc_config(gpio_ctx_t1, appconfGPIO_T1_RPC_PORT, appconfGPIO_RPC_HOST_PRIORITY);
    // #if ON_TILE(0)
    // {
    //     rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
    //     rtos_gpio_start(gpio_ctx_t0);
    //
    //     gpio_test(gpio_ctx_t0);
    //
    // }
    // #endif

    // #if ON_TILE(1)
    // {
    //     rtos_gpio_start(gpio_ctx_t1);
    // }
    // #endif

    // #if ON_TILE(USB_TILE_NO)
    // {
    //     usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
    //     usb_device_control_set_ctx(device_control_usb_ctx, 1);
    //     usb_manager_start(appconfUSB_MGR_TASK_PRIORITY);
    // }
    // #endif

    // #if ON_TILE(AUDIO_TILE)
    // {
    //     const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
    //             PDM_CLOCK_FREQUENCY,
    //             VFE_PIPELINE_AUDIO_SAMPLE_RATE);
    //
    //     rtos_mic_array_start(
    //             mic_array_ctx,
    //             pdm_decimation_factor,
    //             rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
    //             rtos_mic_array_fir_compensation(pdm_decimation_factor),
    //             2 * MIC_DUAL_FRAME_SIZE,
    //             3);
    //
    //     rtos_i2s_start(
    //             i2s_ctx,
    //             rtos_i2s_mclk_bclk_ratio(AUDIO_CLOCK_FREQUENCY, VFE_PIPELINE_AUDIO_SAMPLE_RATE),
    //             I2S_MODE_I2S,
    //             0,
    //             1.2 * VFE_PIPELINE_AUDIO_FRAME_LENGTH,
    //             4);
    //
    //     vfe_pipeline_init(mic_array_ctx, i2s_ctx);
    // }
    // #endif

    test_printf("Done");
    _Exit(0);

    chanend_free(other_tile_c);
    vTaskDelete(NULL);
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1, intertile_ctx, mic_array_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, gpio_ctx, i2s_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    kernel_printf("Start scheduler");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(0) */

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx, mic_array_ctx, i2c_master_ctx, i2c_slave_ctx, qspi_flash_ctx, gpio_ctx, i2s_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    kernel_printf("Start scheduler");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(1) */
