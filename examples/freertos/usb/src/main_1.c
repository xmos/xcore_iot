// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/chanend.h>
#include <rtos_printf.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos/drivers/intertile/api/rtos_intertile.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "demo_main.h"
#include "rtos_usb.h"
#include "rtos_interrupt.h"
#include "usb_support.h"

#if ON_TILE(1)

static rtos_gpio_t gpio_ctx_s;
static rtos_mic_array_t mic_array_ctx_s;

static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;

static rtos_intertile_t intertile_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("vApplicationDaemonTaskStartup() on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());

    rtos_printf("Starting intertile driver\n");
    rtos_intertile_start(intertile_ctx);

    rtos_gpio_rpc_config(gpio_ctx, appconfGPIO_RPC_PORT, appconfGPIO_RPC_HOST_TASK_PRIORITY);
#if OSPREY_BOARD || XCOREAI_EXPLORER
    rtos_mic_array_rpc_config(mic_array_ctx, appconfMIC_ARRAY_RPC_PORT, appconfMIC_ARRAY_RPC_HOST_TASK_PRIORITY);

    const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
            PDM_CLOCK_FREQUENCY,
            48000);

    /* This will be on core 0 */
    rtos_mic_array_interrupt_init(mic_array_ctx);

    rtos_mic_array_start(
            mic_array_ctx,
            pdm_decimation_factor,
            rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
            rtos_mic_array_fir_compensation(pdm_decimation_factor),
            1.2 * MIC_DUAL_FRAME_SIZE,
            configMAX_PRIORITIES-1);
#endif

#if ON_TILE(USB_TILE_NO)
    create_tinyusb_demo(gpio_ctx, appconfTINYUSB_DEMO_TASK_PRIORITY);
    usb_manager_start(appconfUSB_MANAGER_TASK_PRIORITY);
#endif

    vTaskDelete(NULL);
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    TaskHandle_t startup_task;

    board_tile1_init(c0, intertile_ctx, gpio_ctx, mic_array_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

#if ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif

    rtos_printf("Starting startup task on tile 1 with %d stack\n", RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup) * 4);
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

    rtos_printf("Start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(1) */
