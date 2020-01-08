// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "spi_driver.h"

#include "debug_print.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if ( SOC_SPI_PERIPHERAL_USED == 0 )
#define BITSTREAM_SPI_DEVICE_COUNT 0
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
#endif /* SOC_SPI_PERIPHERAL_USED */

static SemaphoreHandle_t xSem_spi;

RTOS_IRQ_ISR_ATTR
void spi_ISR(soc_peripheral_t device)
{
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    status = soc_peripheral_interrupt_status(device);

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
//        soc_dma_ring_buf_t *rx_ring_buf;
//        uint8_t *frame_buffer;
//        int frame_length;
//
//        rx_ring_buf = soc_peripheral_rx_dma_ring_buf(device);
//
//        while( ( frame_buffer = soc_dma_ring_rx_buf_get(rx_ring_buf, &frame_length) ) != NULL )
//        {
//            BaseType_t xResult;
//            xResult = xTimerPendFunctionCallFromISR(
//                    (PendedFunction_t) vDeferredReceive,
//                    frame_buffer,
//                    frame_length,
//                    &xYieldRequired );
//
//            if (xResult != pdPASS)
//            {
//                iptraceETHERNET_RX_EVENT_LOST();
//                debug_printf("eth data lost 3\n", frame_length);
//
//                /* Give the buffer back to the DMA engine */
//                soc_dma_ring_rx_buf_set(rx_ring_buf, frame_buffer, ETH_BUF_SIZE);
//                soc_peripheral_hub_dma_request(device, SOC_DMA_RX_REQUEST);
//            }
//        }
        xSemaphoreGiveFromISR( xSem_spi, &xYieldRequired );
    }

    portEND_SWITCHING_ISR( xYieldRequired );
}

static BaseType_t spi_init( void )
{
    BaseType_t xRetVal;
    xSem_spi = xSemaphoreCreateBinary();

    if( xSem_spi == NULL )
    {
        xRetVal = pdFAIL;
    }
    else
    {
        xRetVal = pdTRUE;
    }

    return xRetVal;
}

soc_peripheral_t spi_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_SPI_DEVICE_COUNT);

    device = bitstream_spi_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            rx_desc_count,
            rx_buf_size,
            tx_desc_count,
            app_data,
            isr_core,
            isr);
    spi_init();
    return device;
}


static void spi_driver_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        uint8_t* tx_buf,
        size_t len,
        int block)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, SPI_DEV_TRANSACTION);

    soc_peripheral_varlist_tx(
            c, 3,
            sizeof(size_t), &len,
            sizeof(uint8_t*), (uint8_t*)&rx_buf,
            sizeof(uint8_t*), (uint8_t*)&tx_buf);

    if( rx_buf != NULL )
    {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);
        soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf, (uint16_t)len);
        soc_peripheral_hub_dma_request(dev, SOC_DMA_RX_REQUEST);
        if( block )
        {
            // replace with semaphore
            while (( soc_dma_ring_rx_buf_get(rx_ring_buf, NULL)) != rx_buf ) {
                ;   /* Wait until we receive data */
            }
        }
    }

    if( tx_buf != NULL )
    {
        soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
        soc_dma_ring_tx_buf_set(tx_ring_buf, tx_buf, (uint16_t)len);
        soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

        uint8_t* tmpbuf;
        while( ( tmpbuf = soc_dma_ring_tx_buf_get( tx_ring_buf, NULL, NULL ) ) != NULL )
        {
            vPortFree(tmpbuf);
        }

        if( block )
        {
            // replace with semaphore
            while (( soc_dma_ring_tx_buf_get(tx_ring_buf, NULL, NULL)) != tx_buf ) {
                ;   /* Wait until the data we just sent is received */
            }
        }
    }
}

void spi_driver_setup(
        soc_peripheral_t dev,
        unsigned speed_in_khz,
        spi_mode_t mode)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, SPI_DEV_SETUP);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(unsigned), &speed_in_khz,
            sizeof(spi_mode_t), &mode);
}

void spi_driver_transmit(
        soc_peripheral_t dev,
        uint8_t* tx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           NULL,
                           tx_buf,
                           len,
                           0);
}

void spi_request(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           rx_buf,
                           NULL,
                           len,
                           0);
}

void spi_transmit_blocking(
        soc_peripheral_t dev,
        uint8_t* tx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           NULL,
                           tx_buf,
                           len,
                           1);
}

void spi_receive_blocking(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           rx_buf,
                           NULL,
                           len,
                           1);
}

void spi_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        uint8_t* tx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           rx_buf,
                           tx_buf,
                           len,
                           0);
}

void spi_transaction_blocking(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        uint8_t* tx_buf,
        size_t len)
{
    spi_driver_transaction(dev,
                           rx_buf,
                           tx_buf,
                           len,
                           1);
}

