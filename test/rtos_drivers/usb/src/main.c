// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos_intertile.h"
#include "usb_support.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "rtos_test/rtos_test_utils.h"
#include "individual_tests/individual_tests.h"

static rtos_intertile_t intertile_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

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
    rtos_intertile_start(intertile_ctx);

    if (RUN_USB_TESTS) {
        if (usb_device_tests(other_tile_c) != 0)
        {
            test_printf("FAIL USB");
        } else {
            test_printf("PASS USB");
        }
    } else {
        test_printf("SKIP USB");
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
                     intertile_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

#if ON_TILE(0)
    if (RUN_USB_TESTS) {
        usb_manager_init();
    }
#endif

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
                     intertile_ctx);
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
