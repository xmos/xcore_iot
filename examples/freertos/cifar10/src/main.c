// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "cifar10_task.h"

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
  configASSERT(0);
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    static rtos_intertile_address_t cifar10_addr_s;
    static rtos_intertile_address_t *cifar10_addr = &cifar10_addr_s;

    cifar10_addr->intertile_ctx = intertile_ctx;
    cifar10_addr->port = CIFAR10_PORT;

#if ON_TILE(0)
    cifar10_app_task_create(cifar10_addr, appconfCIFAR10_TASK_PRIORITY);
#endif

#if ON_TILE(1)
    cifar10_image_classifier_task_create(cifar10_addr, appconfCIFAR10_TASK_PRIORITY);
#endif

	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
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
