// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include <string.h>
#include "soc.h"
#include "bitstream_devices.h"

/* App headers */
#include "app_conf.h"
#include "intertile_ctrl.h"
#include "rpc_test.h"

#if THIS_XCORE_TILE == 0
void soc_tile0_main(
        int tile)
{
    intertile_ctrl_create_t0( appconfINTERTILE_CTRL_TASK_PRIORITY );

    rpc_test_init();

    vTaskStartScheduler();
}
#endif

#if THIS_XCORE_TILE == 1
void soc_tile1_main(
        int tile)
{
    intertile_ctrl_create_t1( appconfINTERTILE_CTRL_TASK_PRIORITY );

    rpc_test_init();

    vTaskStartScheduler();
}
#endif

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
