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


void rx_task(void *arg)
{
    soc_peripheral_t dev = arg;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t recv_msg_buf = msgbuffers->recv_msg_buf;
    for( ;; )
    {
        uint8_t *data = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );
        size_t recv_len = 0;

        rtos_printf("tile[%d] Waiting for message\n", 1&get_local_tile_id());
        recv_len = xMessageBufferReceive(recv_msg_buf, data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
        rtos_printf("tile[%d] Received %d bytes:\n\t<- %s\n", 1&get_local_tile_id(), recv_len, data);

        vPortFree(data);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_msgbuf_recv, device, buf, len, xReturnBufferToDMA)
{
    soc_peripheral_t dev = device;
    BaseType_t xYieldRequired = pdFALSE;
//    intertile_cb_header_t* hdr = (intertile_cb_header_t*)buf;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t recv_msg_buf = msgbuffers->recv_msg_buf;

    uint8_t *payload = buf+1;

    if ( xMessageBufferIsFull(recv_msg_buf) == pdTRUE )
    {
        rtos_printf("tile[%d] Message buffer full, %d bytes lost\n", 1&get_local_tile_id(), len);
        configASSERT(0);    // Buffer was full
    }
    else
    {
        if (xMessageBufferSendFromISR(recv_msg_buf, payload, len, &xYieldRequired) != len)
        {
            configASSERT(0);    // Failed to send full buffer
        }
    }

    *xReturnBufferToDMA = pdTRUE;

    return xYieldRequired;
}

static void intertile_msgbuffer(void *arg)
{
    soc_peripheral_t dev = arg;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t send_msg_buf = msgbuffers->send_msg_buf;

    intertile_cb_header_t msg_hdr;
    intertile_driver_header_init(&msg_hdr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_msgbuf_recv, &msg_hdr);

    uint8_t data[INTERTILE_DEV_BUFSIZE];
    for( ;; )
    {
        size_t recv_len = 0;

//        rtos_printf("tile[%d] Wait for message to send\n", 1&get_local_tile_id());
        recv_len = xMessageBufferReceive(send_msg_buf, &data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
//        rtos_printf("tile[%d] Send %d bytes\n", 1&get_local_tile_id(), recv_len);

        intertile_driver_send_bytes(dev, (uint8_t*)&data, recv_len, &msg_hdr);
    }
}


typedef struct
{
    int id;
    int a;
    int b;
    int ret;
} add_rpc_arg_t ;


#if THIS_XCORE_TILE == 0
static QueueHandle_t eventqueue;
static QueueHandle_t respqueue;

int add(int a, int b)
{
    int ret;
    rtos_printf("tile[%d] Add\n", 1&get_local_tile_id());

    add_rpc_arg_t* msg = pvPortMalloc(sizeof(add_rpc_arg_t));
    msg->id = 12;
    msg->a = a;
    msg->b = b;
    msg->ret = 0;

    xQueueSend( eventqueue, msg, portMAX_DELAY );
    xQueueReceive( respqueue, msg, portMAX_DELAY );
    ret = msg->ret;

    return ret;
}
#endif


#if THIS_XCORE_TILE == 1
int add(int a, int b)
{
    rtos_printf("tile[%d] Add\n", 1&get_local_tile_id());

    return a+b;
}
#endif

#define TEST(a,b) {ret = add(a, b);rtos_printf("%d+%d=%d\n",a, b, ret);}

void add_task(void *arg)
{
    int ret, a, b;
    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(1,1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(-5,2);
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(0,31);
    }
}

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    static intertile_msg_buffers_t msgbuffers;

    MessageBufferHandle_t send_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t recv_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.recv_msg_buf = recv_msg_buf;
    msgbuffers.send_msg_buf = send_msg_buf;


#if THIS_XCORE_TILE == 0
    eventqueue = xQueueCreate(2, INTERTILE_DEV_BUFSIZE);
    respqueue = xQueueCreate(2, INTERTILE_DEV_BUFSIZE);

    msgbuffers.eventQueue = eventqueue;
    msgbuffers.respQueue = respqueue;
#endif

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            &msgbuffers,
            0);

    xTaskCreate(add_task, "add_task", portTASK_STACK_DEPTH(add_task), dev, uxPriority, NULL);

    xTaskCreate(t0_test, "tile0_intertile", portTASK_STACK_DEPTH(t0_test), dev, uxPriority, NULL);
//    xTaskCreate(rx_task, "tile0_task", portTASK_STACK_DEPTH(rx_task), dev, uxPriority, NULL);
    xTaskCreate(intertile_msgbuffer, "tile0_msgbuf", portTASK_STACK_DEPTH(intertile_msgbuffer), dev, uxPriority, NULL);

}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    static intertile_msg_buffers_t msgbuffers;

    MessageBufferHandle_t send_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t recv_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.recv_msg_buf = recv_msg_buf;
    msgbuffers.send_msg_buf = send_msg_buf;

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            &msgbuffers,
            0);

    xTaskCreate(t1_test, "tile1_intertile", portTASK_STACK_DEPTH(t1_test), dev, uxPriority, NULL);
//    xTaskCreate(rx_task, "tile1_task", portTASK_STACK_DEPTH(rx_task), dev, uxPriority, NULL);
    xTaskCreate(intertile_msgbuffer, "tile1_msgbuf", portTASK_STACK_DEPTH(intertile_msgbuffer), dev, uxPriority, NULL);
}
