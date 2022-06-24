// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* App headers */
#include "app_conf.h"
#include "demo_main.h"
#include "usb_support.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char* pcTaskName )
{
    rtos_printf("\n****************************\nStack OF %d %s!\n****************************\\n", THIS_XCORE_TILE, pcTaskName);
    for(;;);
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(USB_TILE_NO)
    usb_manager_start(appconfUSB_MANAGER_TASK_PRIORITY);

#ifdef MSC_MAX_DISKS
    create_tinyusb_disks(qspi_flash_ctx);
#endif

#if DFU_DEMO
    demo_args_t *demo_task_args = pvPortMalloc(sizeof(demo_args_t));
    demo_task_args->gpio_ctx = gpio_ctx_t0;
    demo_task_args->qspi_ctx = qspi_flash_ctx;
    create_tinyusb_demo(demo_task_args, appconfTINYUSB_DEMO_TASK_PRIORITY);
#else
    create_tinyusb_demo(gpio_ctx_t0, appconfTINYUSB_DEMO_TASK_PRIORITY);
#endif
#endif

	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);

#if ON_TILE(USB_TILE_NO)  // usb is not in the default bsp_config
    usb_manager_init();
#endif

    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c0;
    (void)c2;
    (void)c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c1;
    (void)c2;
    (void)c3;

    tile_common_init(c0);
}
#endif
