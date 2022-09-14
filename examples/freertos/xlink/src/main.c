// Copyright 2019-2022 XMOS LIMITED.
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
#include "link_helpers.h"
#include "xlink_rx.h"
#include "xlink_tx.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
    configASSERT(0);
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1)
#if DEMO_TILE == 0
    create_xlink_rx_tasks(appconfXLINK_RX_TASK_PRIORITY);
#else
    create_xlink_tx_tasks(appconfXLINK_TX_TASK_PRIORITY);
#endif
#endif

#if ON_TILE(0)
    /* 1s Heartbeat */
    const rtos_gpio_port_id_t led_port = rtos_gpio_port(PORT_LEDS);
    rtos_gpio_port_enable(gpio_ctx_t0, led_port);
    int led_status=0;

	for (;;) {
        rtos_gpio_port_out(gpio_ctx_t0, led_port, led_status);
        led_status ^= 1;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
#else
    vTaskSuspend(NULL);
#endif
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
