// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_i2s.h"
#include "rtos_intertile.h"
#include "rtos_mic_array.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "rtos_test/rtos_test_utils.h"
#include "individual_tests/individual_tests.h"

static rtos_intertile_t intertile_ctx_s;
static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_i2s_t i2s_master_ctx_s;
static rtos_i2s_t i2s_slave_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_i2s_t *i2s_master_ctx = &i2s_master_ctx_s;
static rtos_i2s_t *i2s_slave_ctx = &i2s_slave_ctx_s;

chanend_t other_tile_c;

#define kernel_printf( FMT, ... )    module_printf("KERNEL", FMT, ##__VA_ARGS__)

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
        if (intertile_device_tests(intertile_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL INTERTILE");
        } else {
            test_printf("PASS INTERTILE");
        }
    } else {
        test_printf("SKIP INTERTILE");
        rtos_intertile_start(intertile_ctx);
    }

    if (RUN_GPIO_TESTS) {
        if (gpio_device_tests(gpio_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL GPIO");
        } else {
            test_printf("PASS GPIO");
        }
    } else {
        test_printf("SKIP GPIO");
    }

    if (RUN_I2C_TESTS) {
        if (i2c_device_tests(i2c_master_ctx, i2c_slave_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL I2C");
        } else {
            test_printf("PASS I2C");
        }
    } else {
        test_printf("SKIP I2C");
    }

    if (RUN_SWMEM_TESTS) {
        if (swmem_device_tests(other_tile_c) != 0)
        {
            test_printf("FAIL SWMEM");
        } else {
            test_printf("PASS SWMEM");
        }
    } else {
        test_printf("SKIP SWMEM");
    }

    if (RUN_QSPI_FLASH_TESTS) {
        if (qspi_flash_device_tests(qspi_flash_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL QSPI_FLASH");
        } else {
            test_printf("PASS QSPI_FLASH");
        }
    } else {
        test_printf("SKIP QSPI_FLASH");
    }

    if (RUN_I2S_TESTS) {
        if (i2s_device_tests(i2s_master_ctx, i2s_slave_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL I2S");
        } else {
            test_printf("PASS I2S");
        }
    } else {
        test_printf("SKIP I2S");
    }

    if (RUN_MIC_ARRAY_TESTS) {
        if (mic_array_device_tests(mic_array_ctx, other_tile_c) != 0)
        {
            test_printf("FAIL MIC_ARRAY");
        } else {
            test_printf("PASS MIC_ARRAY");
        }
    } else {
        test_printf("SKIP MIC_ARRAY");
    }

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
