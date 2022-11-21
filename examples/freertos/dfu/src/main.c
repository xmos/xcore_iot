// Copyright 2022 XMOS LIMITED.
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
#include "usb_support.h"
#include "xcore/channel_streaming.h"
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

void blinky_task(void *arg) {
    uint32_t gpio_port = rtos_gpio_port(PORT_LEDS);
    uint32_t led_val = 0;

    rtos_gpio_port_enable(gpio_ctx_t0, gpio_port);

    for (;;) {
        rtos_gpio_port_out(gpio_ctx_t0, gpio_port, led_val);
        led_val ^= VERSION;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(0)
    rtos_dfu_image_print_debug(dfu_image_ctx);

    uint32_t data;
    rtos_qspi_flash_read(
            qspi_flash_ctx,
            (uint8_t*)&data,
            rtos_dfu_image_get_data_partition_addr(dfu_image_ctx),
            sizeof(int32_t));
    rtos_printf("First word at data partition start is: 0x%x\n", data);
#endif

	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

#if ON_TILE(0)
    xTaskCreate((TaskFunction_t) blinky_task,
                "blinky_task",
                RTOS_THREAD_STACK_SIZE(blinky_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY / 2,
                NULL);
#endif

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
