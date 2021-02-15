// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <xcore/chanend.h>
#include <rtos_printf.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos/drivers/intertile/api/rtos_intertile.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"

#if ON_TILE(1)
static rtos_intertile_t intertile_ctx_s;

static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("Starting intertile driver\n");
    rtos_intertile_start(intertile_ctx);

    vTaskDelete(NULL);
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    rtos_printf("Starting startup task on tile 1 with %d stack\n", RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup) * 4);
    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
				RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
				appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("Start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(1) */
