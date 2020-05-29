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

static intertile_msg_buffers_t msgbuffers;

void rx_task(void *arg)
{
    MessageBufferHandle_t recv_msg_buf = msgbuffers.recv_msg_buf;
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


INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_msgbuf_recv, device, buf, len, status, xReturnBufferToDMA)
{
    BaseType_t xYieldRequired = pdFALSE;
    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM )
    {
        MessageBufferHandle_t recv_msg_buf = msgbuffers.recv_msg_buf;

        uint8_t *payload = buf;

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
    }

    return xYieldRequired;
}

static void intertile_msgbuffer(void *arg)
{
    soc_peripheral_t dev = arg;

    MessageBufferHandle_t send_msg_buf = msgbuffers.send_msg_buf;

    intertile_cb_footer_t msg_ftr;
    intertile_driver_footer_init(&msg_ftr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_msgbuf_recv, &msg_ftr);

    uint8_t *buf;
    for(int i=0; i<appconfINTERTILE_DMA_BUF_CNT; i++)
    {
		buf = pvPortMalloc(INTERTILE_DEV_BUFSIZE);
		soc_dma_ring_rx_buf_set(soc_peripheral_rx_dma_ring_buf(dev), buf, INTERTILE_DEV_BUFSIZE);
    }
    soc_peripheral_hub_dma_request(dev, SOC_DMA_RX_REQUEST);


    uint8_t data[INTERTILE_DEV_BUFSIZE];
    for( ;; )
    {
        size_t recv_len = 0;

//        rtos_printf("tile[%d] Wait for message to send\n", 1&get_local_tile_id());
        recv_len = xMessageBufferReceive(send_msg_buf, &data, INTERTILE_DEV_BUFSIZE, portMAX_DELAY);
//        rtos_printf("tile[%d] Send %d bytes\n", 1&get_local_tile_id(), recv_len);

        intertile_driver_send_bytes(dev, (uint8_t*)&data, recv_len, NULL, 0, &msg_ftr);
    }
}

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    MessageBufferHandle_t send_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t recv_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.recv_msg_buf = recv_msg_buf;
    msgbuffers.send_msg_buf = send_msg_buf;

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
			appconfINTERTILE_DMA_BUF_CNT,
			appconfINTERTILE_DMA_BUF_CNT,
            0);

    xTaskCreate(t0_test, "tile0_intertile", portTASK_STACK_DEPTH(t0_test), &msgbuffers, uxPriority, NULL);
    xTaskCreate(rx_task, "tile0_task", portTASK_STACK_DEPTH(rx_task), NULL, uxPriority, NULL);
    xTaskCreate(intertile_msgbuffer, "tile0_msgbuf", portTASK_STACK_DEPTH(intertile_msgbuffer), dev, uxPriority, NULL);
}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    MessageBufferHandle_t send_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);
    MessageBufferHandle_t recv_msg_buf = xMessageBufferCreate(2 * INTERTILE_DEV_BUFSIZE);

    msgbuffers.recv_msg_buf = recv_msg_buf;
    msgbuffers.send_msg_buf = send_msg_buf;

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
			appconfINTERTILE_DMA_BUF_CNT,
			appconfINTERTILE_DMA_BUF_CNT,
            0);

    xTaskCreate(t1_test, "tile1_intertile", portTASK_STACK_DEPTH(t1_test), &msgbuffers, uxPriority, NULL);
    xTaskCreate(rx_task, "tile1_task", portTASK_STACK_DEPTH(rx_task), NULL, uxPriority, NULL);
    xTaskCreate(intertile_msgbuffer, "tile1_msgbuf", portTASK_STACK_DEPTH(intertile_msgbuffer), dev, uxPriority, NULL);
}
