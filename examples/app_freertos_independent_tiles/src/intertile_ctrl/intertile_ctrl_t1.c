// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_ctrl.h"


void t1_test(void *arg)
{
//    soc_peripheral_t dev = arg;
//
//    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
//    MessageBufferHandle_t send_msg_buf = msgbuffers->send_msg_buf;
//    MessageBufferHandle_t recv_msg_buf = msgbuffers->recv_msg_buf;
//
//    uint8_t buf[] = "Hi Tile 0";
//    uint8_t buf1[] = "Bye Tile 0";
//    size_t len;
//
//    for( ;; )
//    {
//        uint8_t *data = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );
//        size_t recv_len = 0;
//
//        rtos_printf("tile[%d] Waiting for message\n", 1&get_local_tile_id());
//        recv_len = xMessageBufferReceive(recv_msg_buf, data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
//
//        add_rpc_arg_t* msg = (add_rpc_arg_t*)data;
//        switch(msg->id)
//        {
//            case 12:
//                (msg->ret) = add(msg->a, msg->b);
//                break;
//            default:
//                rtos_printf("INVALID CASE\n");
//        }
//
//        rtos_printf("msg->ret = %d\n", msg->ret);
//        if( xMessageBufferSend( send_msg_buf, (void*)data, recv_len, portMAX_DELAY) != recv_len )
//        {
//            configASSERT(0);    // Failed to send full buffer
//        }
//
//        vPortFree(data);
//    }
}
