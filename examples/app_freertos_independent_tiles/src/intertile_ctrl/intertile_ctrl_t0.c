// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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
//    soc_peripheral_t dev = arg;
//
//    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
//    MessageBufferHandle_t send_msg_buf = msgbuffers->send_msg_buf;
//    MessageBufferHandle_t recv_msg_buf = msgbuffers->recv_msg_buf;
//    QueueHandle_t eventQueue = msgbuffers->eventQueue;
//    QueueHandle_t respQueue = msgbuffers->respQueue;
//
//    uint8_t buf[] = "Hello Tile 1";
//    uint8_t buf1[] = "Goodbye Tile 1";
//    size_t len;
//
//    for( ;; )
//    {
//        uint8_t *data = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );
//        xQueueReceive( eventQueue, data, portMAX_DELAY );
//        len = sizeof(add_rpc_arg_t);
//
//        if( xMessageBufferSend( send_msg_buf, (void*)data, len, portMAX_DELAY) != len )
//        {
//            configASSERT(0);    // Failed to send full buffer
//        }
//        size_t recv_len = 0;
//
//        rtos_printf("tile[%d] Waiting for message\n", 1&get_local_tile_id());
//        recv_len = xMessageBufferReceive(recv_msg_buf, data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
//
//        rtos_printf("data is:%d\n", ((add_rpc_arg_t*)data)->ret);
//        xQueueSend(respQueue, data, portMAX_DELAY );
//
//        vPortFree(data);
//    }
}
