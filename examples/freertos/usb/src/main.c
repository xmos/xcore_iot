// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"


void vApplicationCoreInitHook(BaseType_t xCoreID)
{
#if ON_TILE(0)
    rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif /* ON_TILE(0) */

#if ON_TILE(1)
    rtos_printf("Initializing tile 1, core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif /* ON_TILE(1) */
}

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
