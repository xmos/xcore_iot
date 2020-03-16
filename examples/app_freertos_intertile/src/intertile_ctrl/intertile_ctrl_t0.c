// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_ctrl.h"


void t0_test(void *arg)
{
    soc_peripheral_t dev = arg;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t xMessageBufferSend = msgbuffers->xMessageBufferSend;

    uint8_t buf[] = "Hello Tile 1";
    uint8_t buf1[] = "Goodbye Tile 1";
    size_t len;

    for( ;; )
    {
        len = strlen((char *)buf) + 1;
        rtos_printf("tile[%d] Send %d bytes:\n\t-> %s\n", 1&get_local_tile_id(), len, buf);
        if( xMessageBufferSend( xMessageBufferSend, (void*)buf, len, portMAX_DELAY) != len )
        {
            configASSERT(0);    // Failed to send full buffer
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        len = strlen((char *)buf1) + 1;
        rtos_printf("tile[%d] Send %d bytes:\n\t-> %s\n", 1&get_local_tile_id(), len, buf);
        if( xMessageBufferSend( xMessageBufferSend, (void*)buf1, len, portMAX_DELAY) != len )
        {
            configASSERT(0);    // Failed to send full buffer
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
