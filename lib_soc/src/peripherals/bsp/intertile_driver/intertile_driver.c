// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "intertile_driver.h"

#include "FreeRTOS.h"

#if ( SOC_INTERTILE_PERIPHERAL_USED == 0 )
#define BITSTREAM_INTERTILE_DEVICE_COUNT 0
soc_peripheral_t bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_COUNT];
#endif /* SOC_INTERTILE_PERIPHERAL_USED */

typedef struct intertile_isr_callback {
    INTERTILE_ISR_CALLBACK_ATTR intertile_isr_cb_t cb;
} intertile_isr_callback_t;

static intertile_isr_callback_t intertile_isr_callback_map[ BITSTREAM_INTERTILE_DEVICE_COUNT ][ INTERTILE_DEV_HANDLER_COUNT ];

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_null, device, buf, len, xReturnBufferToDMA)
{
    (void) device;
    (void) buf;
    (void) len;

    return pdFALSE;
}

RTOS_IRQ_ISR_ATTR
static void intertile_isr( soc_peripheral_t device )
{
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;
    int device_id = soc_peripheral_get_id( device );

    status = soc_peripheral_interrupt_status(device);

    if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM) {
        soc_dma_ring_buf_t *tx_ring_buf;
        int more;
        tx_ring_buf = soc_peripheral_tx_dma_ring_buf(device);
        do
        {
            soc_dma_ring_tx_buf_get(tx_ring_buf, NULL, &more);   // give the buffer back for now
        } while(more);
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        soc_dma_ring_buf_t *rx_ring_buf;
        uint8_t *frame_buffer;
        int frame_length;

        rx_ring_buf = soc_peripheral_rx_dma_ring_buf(device);

        while( ( frame_buffer = soc_dma_ring_rx_buf_get(rx_ring_buf, &frame_length, NULL) ) != NULL )
        {
            BaseType_t xReturnBufferToDMA = pdFALSE;

            uint32_t cb_id = frame_buffer[0];
            intertile_isr_callback_t mapped_cb;

            if( ( mapped_cb.cb = intertile_isr_callback_map[ device_id ][ cb_id ].cb ) != NULL )
            {
                if( mapped_cb.cb( device, frame_buffer, frame_length, &xReturnBufferToDMA ) == pdTRUE )
                {
                    xYieldRequired = pdTRUE;
                }
            }

            if (xReturnBufferToDMA == pdTRUE)
            {
                /* Give the buffer back to the DMA engine */
                soc_dma_ring_rx_buf_set(rx_ring_buf, frame_buffer, INTERTILE_DEV_BUFSIZE);
            }
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

BaseType_t intertile_driver_header_init(
        intertile_cb_header_t* cb_header,
        intertile_cb_id_t cb_id)
{
    BaseType_t xRetVal = pdPASS;

    if( cb_header != NULL )
    {
        cb_header->cb_id = cb_id;
    }
    else
    {
        xRetVal = pdFAIL;
    }

    return xRetVal;
}

BaseType_t intertile_driver_register_callback(
        soc_peripheral_t dev,
        intertile_isr_cb_t isr_cb,
        intertile_cb_header_t* cb_header)
{
    BaseType_t xRetVal = pdFAIL;
    int device_id = soc_peripheral_get_id( dev );

    if( ( cb_header->cb_id < INTERTILE_DEV_HANDLER_COUNT ) && ( device_id >= 0 ) && ( device_id < BITSTREAM_INTERTILE_DEVICE_COUNT ) )
    {
        intertile_isr_callback_map[ device_id ][ cb_header->cb_id ].cb = (isr_cb != NULL) ? isr_cb : intertile_dev_null;
        xRetVal = pdPASS;
    }

    return xRetVal;
}

BaseType_t intertile_driver_unregister_callback(
        soc_peripheral_t dev,
        intertile_cb_header_t* cb_header)
{
    BaseType_t xRetVal = pdFAIL;
    int device_id = soc_peripheral_get_id( dev );

    if( ( device_id >= 0 ) && ( device_id < BITSTREAM_INTERTILE_DEVICE_COUNT ) )
    {
        intertile_isr_callback_map[ device_id ][ cb_header->cb_id ].cb = NULL;
        xRetVal = pdPASS;
    }

    return xRetVal;
}

void intertile_driver_send_bytes(
        soc_peripheral_t dev,
        uint8_t *bytes,
        unsigned len,
        intertile_cb_header_t* cb_header)
{
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

    configASSERT( (len + sizeof(intertile_cb_header_t)) <= INTERTILE_DEV_BUFSIZE );

    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, bytes, len, 1, 2);
    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, cb_header, sizeof(intertile_cb_header_t), 0, 2);

    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);
}

soc_peripheral_t intertile_driver_init(
        int device_id,
        int rx_desc_count,
        int tx_desc_count,
        void *app_data,
        int isr_core)
{
    soc_peripheral_t device;

    configASSERT(device_id >= 0 && device_id < BITSTREAM_INTERTILE_DEVICE_COUNT);

    device = bitstream_intertile_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            rx_desc_count,
            INTERTILE_DEV_BUFSIZE,
            tx_desc_count,
            app_data,
            isr_core,
            (rtos_irq_isr_t)intertile_isr);

    return device;
}
