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



void rx_task(void *arg)
{
    soc_peripheral_t dev = arg;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t xMessageBufferSend = msgbuffers->xMessageBufferSend;
    MessageBufferHandle_t xMessageBufferRecv = msgbuffers->xMessageBufferRecv;
    for( ;; )
    {
        uint8_t *data = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );
        size_t recv_len = 0;

        rtos_printf("tile[%d] Wait for message\n", 1&get_local_tile_id());
        recv_len = xMessageBufferReceive(xMessageBufferRecv, data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
        rtos_printf("tile[%d] Received %d bytes:\n\t%s\n", 1&get_local_tile_id(), recv_len, data);

        vPortFree(data);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_test0, device, buf, len, xReturnBufferToDMA)
{
    soc_peripheral_t dev = device;
    BaseType_t xYieldRequired = pdFALSE;
    intertile_cb_header_t* tmp = (intertile_cb_header_t*)buf;

    intertile_msg_buffers_t* msgbuffers = (intertile_msg_buffers_t*)soc_peripheral_app_data( dev );
    MessageBufferHandle_t xMessageBufferSend = msgbuffers->xMessageBufferSend;
    MessageBufferHandle_t xMessageBufferRecv = msgbuffers->xMessageBufferRecv;

    uint8_t *payload = buf+1;

    if ( xMessageBufferIsFull(xMessageBufferRecv) == pdTRUE )
    {
        rtos_printf("tile[%d] Message buffer full, %d bytes lost\n", 1&get_local_tile_id(), len);
        configASSERT(0);    // Buffer was full
    }
    else
    {
        if (xMessageBufferSendFromISR(xMessageBufferRecv, payload, len, &xYieldRequired) != len)
        {
            configASSERT(0);    // Failed to send full buffer
        }
    }

    *xReturnBufferToDMA = pdTRUE;

    rtos_printf("tile[%d] rx %d header id:%d bytes:%s\n",
            1&get_local_tile_id(),
            len, tmp->cb_id, buf+1);

    return xYieldRequired;
}

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_test1, device, buf, len, xReturnBufferToDMA)
{
    (void) device;
    BaseType_t xYieldRequired = pdFALSE;
    intertile_cb_header_t* tmp = (intertile_cb_header_t*)buf;

    rtos_printf("tile[%d] rx %d header id:%d bytes:%s\n",
            1&get_local_tile_id(),
            len, tmp->cb_id, buf+1);

    *xReturnBufferToDMA = pdTRUE;

    return xYieldRequired;
}


void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    static intertile_msg_buffers_t msgbuffers;

    MessageBufferHandle_t xMessageBufferSend = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t xMessageBufferRecv = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.xMessageBufferRecv = xMessageBufferRecv;
    msgbuffers.xMessageBufferSend = xMessageBufferSend;

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            &msgbuffers,
            0);

    xTaskCreate(t0_test, "tile0_intertile", portTASK_STACK_DEPTH(t0_test), dev, uxPriority, NULL);
    xTaskCreate(rx_task, "tile0_task", portTASK_STACK_DEPTH(rx_task), dev, uxPriority, NULL);
}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    static intertile_msg_buffers_t msgbuffers;

    MessageBufferHandle_t xMessageBufferSend = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t xMessageBufferRecv = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.xMessageBufferRecv = xMessageBufferRecv;
    msgbuffers.xMessageBufferSend = xMessageBufferSend;

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            &msgbuffers,
            0);

    xTaskCreate(t1_test, "tile1_intertile", portTASK_STACK_DEPTH(t1_test), dev, uxPriority, NULL);
    xTaskCreate(rx_task, "tile0_task", portTASK_STACK_DEPTH(rx_task), dev, uxPriority, NULL);
}
