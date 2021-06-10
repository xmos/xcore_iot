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
static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_i2s_t i2s_master_ctx_s;
static rtos_i2s_t i2s_slave_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_i2s_t *i2s_master_ctx = &i2s_master_ctx_s;
static rtos_i2s_t *i2s_slave_ctx = &i2s_slave_ctx_s;

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
    /* Intertile test must always before any test that uses RPC or the intertile
       device must be started */
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

    if (RUN_SWMEM_TESTS) {
        swmem_device_tests(other_tile_c);
    } else {
        test_printf("Skipped SWMEM tests");
    }

    if (RUN_QSPI_FLASH_TESTS) {
        qspi_flash_device_tests(qspi_flash_ctx, other_tile_c);
    } else {
        test_printf("Skipped QSPI_FLASH tests");
    }

    if (RUN_I2S_TESTS) {
        i2s_device_tests(i2s_master_ctx, i2s_slave_ctx, other_tile_c);
    } else {
        test_printf("Skipped I2S tests");
    }

    if (RUN_MIC_ARRAY_TESTS) {
        mic_array_device_tests(mic_array_ctx, other_tile_c);
    } else {
        test_printf("Skipped MIC_ARRAY tests");
    }

    test_printf("Done");
    _Exit(0);

    chanend_free(other_tile_c);
    vTaskDelete(NULL);
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1,
                     intertile_ctx,
                     mic_array_ctx,
                     i2c_master_ctx,
                     spi_master_ctx,
                     qspi_flash_ctx,
                     gpio_ctx,
                     i2s_master_ctx,
                     i2s_slave_ctx);
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
    board_tile1_init(c0,
                     intertile_ctx,
                     mic_array_ctx,
                     i2c_master_ctx,
                     i2c_slave_ctx,
                     qspi_flash_ctx,
                     gpio_ctx,
                     i2s_master_ctx);
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
