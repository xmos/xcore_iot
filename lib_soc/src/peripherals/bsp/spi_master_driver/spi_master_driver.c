// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "spi_master_driver.h"

#include "debug_print.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if ( SOC_SPI_PERIPHERAL_USED == 0 )
#define BITSTREAM_SPI_DEVICE_COUNT 0
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
#endif /* SOC_SPI_PERIPHERAL_USED */


RTOS_IRQ_ISR_ATTR
void spi_master_isr(soc_peripheral_t device)
{
    QueueHandle_t queue = soc_peripheral_app_data(device);
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    status = soc_peripheral_interrupt_status(device);

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        soc_dma_ring_buf_t *rx_ring_buf;
        int length;
        uint8_t *rx_buf;

        configASSERT(device == bitstream_spi_devices[BITSTREAM_SPI_DEVICE_A]);

        rx_ring_buf = soc_peripheral_rx_dma_ring_buf(device);
        rx_buf = soc_dma_ring_rx_buf_get(rx_ring_buf, &length);
        configASSERT(rx_buf != NULL);

        if (xQueueSendFromISR(queue, &rx_buf, &xYieldRequired) == errQUEUE_FULL) {
            ;   // We do not set a new buffer
        }
    }

    portEND_SWITCHING_ISR( xYieldRequired );
}


soc_peripheral_t spi_master_driver_init(
        int device_id,
        int isr_core)
{
    soc_peripheral_t device;

    QueueHandle_t queue;
    queue = xQueueCreate(1, sizeof(void *));

    xassert(device_id >= 0 && device_id < BITSTREAM_SPI_DEVICE_COUNT);

    device = bitstream_spi_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            1,
            0,
            1,
            queue,
            isr_core,
            (rtos_irq_isr_t) spi_master_isr);

    return device;
}


static void spi_driver_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t rx_len,
        uint8_t* tx_buf,
        size_t tx_len)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    if( rx_buf != NULL )
    {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);
        soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf, (uint16_t)rx_len );
    }

    if( tx_buf != NULL )
    {
        soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
        soc_dma_ring_tx_buf_set(tx_ring_buf, tx_buf, (uint16_t)tx_len );
    }
    else
    {
        soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
        soc_dma_ring_tx_buf_set(tx_ring_buf, &NULL[1], 0 );
#warning update once framework is updated to accept NULL and length of 0
        // TODO: replace with NULL, 0 with updates to soc_dma...
    }

    soc_peripheral_function_code_tx(c, SPI_MASTER_DEV_TRANSACTION);

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(size_t), &rx_len);
}

void spi_master_device_init(
        soc_peripheral_t dev,
        unsigned cs_port_bit,
        unsigned cpol,
        unsigned cpha,
        unsigned clock_divide,
        unsigned cs_to_data_delay_ns,
        unsigned byte_setup_ns)
{
    chanend c_ctrl = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c_ctrl, SPI_MASTER_DEV_INIT);

    soc_peripheral_varlist_tx(
            c_ctrl, 6,
            sizeof(unsigned), &cs_port_bit,
            sizeof(unsigned), &cpol,
            sizeof(unsigned), &cpha,
            sizeof(unsigned), &clock_divide,
            sizeof(unsigned), &cs_to_data_delay_ns,
            sizeof(unsigned), &byte_setup_ns);
}


void spi_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t rx_len,
        uint8_t* tx_buf,
        size_t tx_len)
{
    uint8_t* tmpbuf;
    QueueHandle_t queue;
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

    if(tx_buf == NULL) { tx_len = 0; }
    if(rx_buf == NULL) { rx_len = 0; }

    spi_driver_transaction(dev,
                           rx_buf, rx_len,
                           tx_buf, tx_len);

    /* Wait until tx_buf has been transferred */
    while( ( tmpbuf = soc_dma_ring_tx_buf_get( tx_ring_buf, NULL, NULL ) ) != tx_buf ) {;}

    if(rx_buf > 0)
    {
        queue = soc_peripheral_app_data(dev);
        xQueueReceive(queue, &rx_buf, portMAX_DELAY);
    }
}
