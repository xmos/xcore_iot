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

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_null, device, buf, len, status, xReturnBufferToDMA)
{
    (void) device;
    (void) buf;
    (void) len;
    (void) status;
    (void) xReturnBufferToDMA;

    return pdFALSE;
}

RTOS_IRQ_ISR_ATTR
static void intertile_isr( soc_peripheral_t device )
{
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;
    int device_id = soc_peripheral_get_id( device );

    status = soc_peripheral_interrupt_status(device);

    if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM)
    {
//        rtos_printf("tile[%d] dma tx done\n", 1&get_local_tile_id());
        soc_dma_ring_buf_t *tx_ring_buf;
        uint8_t *ftr_buf, *opt_ftr, *bytes_buf;
        int ftr_buf_len, opt_ftr_len, bytes_buf_len;
        int more;

        tx_ring_buf = soc_peripheral_tx_dma_ring_buf(device);
        while( ( bytes_buf = soc_dma_ring_tx_buf_get(tx_ring_buf, &bytes_buf_len, &more) ) != NULL )
        {
            xassert(more);
            opt_ftr = soc_dma_ring_tx_buf_get(tx_ring_buf, &opt_ftr_len, &more); xassert(more);
            ftr_buf = soc_dma_ring_tx_buf_get(tx_ring_buf, &ftr_buf_len, &more); xassert(!more);

            uint32_t cb_id = ftr_buf[0];
            intertile_isr_callback_t mapped_cb;

            if( ( mapped_cb.cb = intertile_isr_callback_map[ device_id ][ cb_id ].cb ) != NULL )
            {
                if( mapped_cb.cb( device, bytes_buf, bytes_buf_len, SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM, NULL ) == pdTRUE )
                {
                    xYieldRequired = pdTRUE;
                }
            }
        }
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM)
    {
//        rtos_printf("tile[%d] dma rx done\n", 1&get_local_tile_id());
        soc_dma_ring_buf_t *rx_ring_buf;
        uint8_t *frame_buffer;
        int frame_length;

        rx_ring_buf = soc_peripheral_rx_dma_ring_buf(device);

        while( ( frame_buffer = soc_dma_ring_rx_buf_get(rx_ring_buf, &frame_length, NULL) ) != NULL )
        {
            BaseType_t xReturnBufferToDMA = pdFALSE;
            intertile_isr_callback_t mapped_cb;

            /* cb_id is located at the start of the footer */
            uint32_t cb_id = frame_buffer[ frame_length - sizeof( intertile_cb_footer_t ) ];

            if( ( mapped_cb.cb = intertile_isr_callback_map[ device_id ][ cb_id ].cb ) != NULL )
            {
                if( mapped_cb.cb( device, frame_buffer, frame_length, SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM, &xReturnBufferToDMA ) == pdTRUE )
                {
                    xYieldRequired = pdTRUE;
                }
            }

            if (xReturnBufferToDMA == pdTRUE)
            {
                /* Give the buffer back to the DMA engine */
                soc_dma_ring_rx_buf_set(rx_ring_buf, frame_buffer, INTERTILE_DEV_BUFSIZE);
                soc_peripheral_hub_dma_request(device, SOC_DMA_RX_REQUEST);
            }
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

BaseType_t intertile_driver_footer_init(
        intertile_cb_footer_t* cb_footer,
        intertile_cb_id_t cb_id)
{
    BaseType_t xRetVal = pdPASS;

    if( cb_footer != NULL )
    {
        cb_footer->cb_id = cb_id;
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
        intertile_cb_footer_t* cb_footer)
{
    BaseType_t xRetVal = pdFAIL;
    int device_id = soc_peripheral_get_id( dev );

    if( ( cb_footer->cb_id < INTERTILE_DEV_HANDLER_COUNT ) && ( device_id >= 0 ) && ( device_id < BITSTREAM_INTERTILE_DEVICE_COUNT ) )
    {
        intertile_isr_callback_map[ device_id ][ cb_footer->cb_id ].cb = (isr_cb != NULL) ? isr_cb : intertile_dev_null;
        xRetVal = pdPASS;
    }

    return xRetVal;
}

BaseType_t intertile_driver_unregister_callback(
        soc_peripheral_t dev,
        intertile_cb_footer_t* cb_footer)
{
    BaseType_t xRetVal = pdFAIL;
    int device_id = soc_peripheral_get_id( dev );

    if( ( device_id >= 0 ) && ( device_id < BITSTREAM_INTERTILE_DEVICE_COUNT ) )
    {
        intertile_isr_callback_map[ device_id ][ cb_footer->cb_id ].cb = NULL;
        xRetVal = pdPASS;
    }

    return xRetVal;
}

void intertile_driver_send_bytes(
        soc_peripheral_t dev,
        uint8_t *bytes,
        unsigned len,
        uint8_t *opt_footer,
        unsigned opt_len,
        intertile_cb_footer_t* cb_footer)
{
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

    configASSERT( (len + opt_len + sizeof(intertile_cb_footer_t)) <= INTERTILE_DEV_BUFSIZE );

    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, cb_footer, sizeof(intertile_cb_footer_t), 2, 3);
    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, opt_footer, opt_len, 1, 3);
    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, bytes, len, 0, 3);

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
            0,
            tx_desc_count,
            app_data,
            isr_core,
            (rtos_irq_isr_t)intertile_isr);

    return device;
}
