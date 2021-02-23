// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <stdbool.h>
#include <platform.h>
#include <xs1.h>
#include <xcore/triggerable.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "demo_main.h"
#include "rtos_usb.h"
#include "rtos_interrupt.h"
#include "usb_support.h"
#include "fs_support.h"

#if ON_TILE(0)

static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_mic_array_t mic_array_ctx_s;
static rtos_intertile_t intertile_ctx_s;

static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("vApplicationDaemonTaskStartup() on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());

    rtos_printf("Starting intertile driver\n");
    rtos_intertile_start(intertile_ctx);

    rtos_gpio_rpc_config(gpio_ctx, appconfGPIO_RPC_PORT, appconfGPIO_RPC_HOST_TASK_PRIORITY);
#if OSPREY_BOARD || XCOREAI_EXPLORER
    rtos_mic_array_rpc_config(mic_array_ctx, appconfMIC_ARRAY_RPC_PORT, appconfMIC_ARRAY_RPC_HOST_TASK_PRIORITY);
#endif

    /* Initialize drivers  */
    rtos_printf("Starting GPIO driver\n");
    rtos_gpio_start(gpio_ctx);

    rtos_printf("Starting QSPI flash driver\n");
    rtos_qspi_flash_start(qspi_flash_ctx, appconfRTOS_QSPI_FLASH_TASK_PRIORITY);

#if ON_TILE(USB_TILE_NO)
    rtos_printf("Starting filesystem\n");
    rtos_fatfs_init(qspi_flash_ctx);        // TODO add rpc qspi flash when not on the same tile

#ifdef MSC_MAX_DISKS        // temporary
    create_tinyusb_disks(qspi_flash_ctx);
#endif

    create_tinyusb_demo(gpio_ctx, appconfTINYUSB_DEMO_TASK_PRIORITY);
    usb_manager_start(appconfUSB_MANAGER_TASK_PRIORITY);
#endif

    vTaskDelete(NULL);
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    TaskHandle_t startup_task;

    (void) c0;
    board_tile0_init(c1, intertile_ctx, qspi_flash_ctx, gpio_ctx, mic_array_ctx);
    (void) c2;
    (void) c3;

    rtos_printf("Starting startup task on tile 0 with %d stack\n", RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup) * 4);
    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
				RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
				appconfSTARTUP_TASK_PRIORITY,
                &startup_task);

    /*
     * Force the startup task to be on core 0. Any interrupts that it
     * starts will therefore be enabled on core 0 and will not conflict
     * with the XUD task.
     */
    vTaskCoreExclusionSet(startup_task, ~(1 << 0));

    rtos_printf("Start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

#endif /* ON_TILE(0) */
