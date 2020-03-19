// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include <string.h>
#include "soc.h"
#include "bitstream_devices.h"
#include "intertile_ctrl.h"

/* App headers */
#include "app_conf.h"

void dup_test(int a)
{
    debug_printf("Hello from dup test on tile 0 (%d)\n", a);
}

void soc_tile0_main(
        int tile)
{
    intertile_ctrl_create_t0( appconfINTERTILE_CTRL_TASK_PRIORITY );

    dup_test(42);

    vTaskStartScheduler();
}

void soc_tile1_main(
        int tile)
{
    intertile_ctrl_create_t1( appconfINTERTILE_CTRL_TASK_PRIORITY );

    vTaskStartScheduler();
}

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
