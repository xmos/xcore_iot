// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "intertile_driver.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if ( SOC_INTERTILE_PERIPHERAL_USED == 0 )
#define BITSTREAM_INTERTILE_DEVICE_COUNT 0
soc_peripheral_t bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_COUNT];
#endif /* SOC_INTERTILE_PERIPHERAL_USED */

typedef struct intertile_isr_callback {
    INTERTILE_ISR_CALLBACK_ATTR intertile_isr_cb_t cb;
} intertile_isr_callback_t;

typedef struct {
    soc_peripheral_t dev;
    SemaphoreHandle_t lock;
    intertile_isr_callback_t intertile_isr_callback_map[ INTERTILE_DEV_HANDLER_COUNT ];
} intertile_driver_t;

RTOS_IRQ_ISR_ATTR
static void intertile_isr( soc_peripheral_t device )
{
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;
    intertile_driver_t *driver_struct = (intertile_driver_t *) soc_peripheral_app_data(device);

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
            configASSERT(more);
            opt_ftr = soc_dma_ring_tx_buf_get(tx_ring_buf, &opt_ftr_len, &more); configASSERT(more);
            ftr_buf = soc_dma_ring_tx_buf_get(tx_ring_buf, &ftr_buf_len, &more); configASSERT(!more);

            intertile_cb_id_t cb_id = ftr_buf[0];
            intertile_isr_callback_t mapped_cb;

            configASSERT( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) );
            if( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) )
            {
                if( ( mapped_cb.cb = driver_struct->intertile_isr_callback_map[ cb_id ].cb ) != NULL )
                {
                    if( mapped_cb.cb( device, bytes_buf, bytes_buf_len, SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM, NULL ) == pdTRUE )
                    {
                        xYieldRequired = pdTRUE;
                    }
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
            BaseType_t xReturnBufferToDMA = pdTRUE;
            intertile_isr_callback_t mapped_cb;

            /* cb_id is located at the start of the footer */
            intertile_cb_id_t cb_id = frame_buffer[ frame_length - sizeof( intertile_cb_footer_t ) ];

            configASSERT( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) );
            if( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) )
            {
                if( ( mapped_cb.cb = driver_struct->intertile_isr_callback_map[ cb_id ].cb ) != NULL )
                {
                    if( mapped_cb.cb( device, frame_buffer, frame_length, SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM, &xReturnBufferToDMA ) == pdTRUE )
                    {
                        xYieldRequired = pdTRUE;
                    }
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
    intertile_cb_id_t cb_id = cb_footer->cb_id;
    intertile_driver_t *driver_struct = (intertile_driver_t *) soc_peripheral_app_data(dev);

    if( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) )
    {
        driver_struct->intertile_isr_callback_map[ cb_id ].cb = isr_cb;
        xRetVal = pdPASS;
    }

    return xRetVal;
}

BaseType_t intertile_driver_unregister_callback(
        soc_peripheral_t dev,
        intertile_cb_footer_t* cb_footer)
{
    BaseType_t xRetVal = pdFAIL;
    intertile_cb_id_t cb_id = cb_footer->cb_id;
    intertile_driver_t *driver_struct = (intertile_driver_t *) soc_peripheral_app_data(dev);

    if( ( cb_id >= 0 ) && ( cb_id < INTERTILE_DEV_HANDLER_COUNT ) )
    {
        driver_struct->intertile_isr_callback_map[ cb_id ].cb = NULL;
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
    intertile_driver_t *driver_struct = (intertile_driver_t *) soc_peripheral_app_data(dev);

    configASSERT( (len + opt_len + sizeof(intertile_cb_footer_t)) <= INTERTILE_DEV_BUFSIZE );

    xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, cb_footer, sizeof(intertile_cb_footer_t), 2, 3);
    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, opt_footer, opt_len, 1, 3);
    soc_dma_ring_tx_buf_sg_set(tx_ring_buf, bytes, len, 0, 3);

    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    xSemaphoreGive(driver_struct->lock);
}

soc_peripheral_t intertile_driver_init(
        int device_id,
        int rx_desc_count,
        int tx_desc_count,
        int isr_core)
{
    soc_peripheral_t device;
    intertile_driver_t *driver_struct;

    configASSERT(device_id >= 0 && device_id < BITSTREAM_INTERTILE_DEVICE_COUNT);

    device = bitstream_intertile_devices[device_id];

    driver_struct = pvPortMalloc(sizeof(intertile_driver_t));

    driver_struct->lock = xSemaphoreCreateMutex();
    memset(driver_struct->intertile_isr_callback_map, 0, sizeof(driver_struct->intertile_isr_callback_map));
    driver_struct->dev = device;

    soc_peripheral_common_dma_init(
            device,
            rx_desc_count,
            0,
            tx_desc_count,
            driver_struct,
            isr_core,
            (rtos_irq_isr_t)intertile_isr);

    return device;
}
