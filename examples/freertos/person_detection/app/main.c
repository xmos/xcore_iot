// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */

/* App headers */
#include "soc.h"
#include "bitstream_devices.h"
#include "spi_camera.h"
#include "queue_to_ai.h"
#include "app_conf.h"

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

void soc_tile0_main(
        int tile)
{
    vTaskStartScheduler();
}

void vApplicationDaemonTaskStartupHook( void )
{
    QueueHandle_t qcam2ai = xQueueCreate( 1, sizeof( uint8_t* ) );

    if( create_spi_camera_to_queue( appconfSPI_CAMERA_TASK_PRIORITY, qcam2ai )
        == pdTRUE )
    {
        create_queue_to_ai( appconfQUEUE_TO_AI_TASK_PRIORITY, qcam2ai );
    }
    else
    {
        debug_printf("Camera setup failed...\n");
    }
}

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
}
