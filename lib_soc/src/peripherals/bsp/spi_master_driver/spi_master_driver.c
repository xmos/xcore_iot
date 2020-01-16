// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "spi_master_driver.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if ( SOC_SPI_PERIPHERAL_USED == 0 )
#define BITSTREAM_SPI_DEVICE_COUNT 0
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
#endif /* SOC_SPI_PERIPHERAL_USED */


RTOS_IRQ_ISR_ATTR
static void spi_master_isr(soc_peripheral_t dev)
{
    SemaphoreHandle_t sem = (SemaphoreHandle_t) soc_peripheral_app_data(dev);
    soc_dma_ring_buf_t *ring_buf;
    int more;
    BaseType_t xYieldRequired = pdFALSE;
    BaseType_t ret;
    uint32_t status;

    status = soc_peripheral_interrupt_status(dev);

    if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM) {
        ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
        ret = xSemaphoreGiveFromISR(sem, &xYieldRequired);
        configASSERT(ret == pdTRUE);
        do {
            soc_dma_ring_tx_buf_get(ring_buf, NULL, &more);
        } while (more);
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        ring_buf = soc_peripheral_rx_dma_ring_buf(dev);
        ret = xSemaphoreGiveFromISR(sem, &xYieldRequired);
        configASSERT(ret == pdTRUE);
        do {
            soc_dma_ring_rx_buf_get(ring_buf, NULL, &more);
        } while (more);
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

soc_peripheral_t spi_master_driver_init(
        int device_id,
        int dma_buffer_count,
        int isr_core)
{
    soc_peripheral_t device;

    SemaphoreHandle_t sem;
    sem = xSemaphoreCreateCounting(2, 0);

    xassert(device_id >= 0 && device_id < BITSTREAM_SPI_DEVICE_COUNT);

    device = bitstream_spi_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            dma_buffer_count,
            0,
            dma_buffer_count,
            sem,
            isr_core,
            (rtos_irq_isr_t) spi_master_isr);

    return device;
}

static void spi_driver_transaction(
        soc_peripheral_t dev,
        uint8_t *rx_buf,
        size_t rx_len,
        uint8_t *tx_buf,
        size_t tx_len)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

    if (rx_len > 0) {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

        soc_peripheral_function_code_tx(c, SPI_MASTER_DEV_TRANSACTION);

        soc_peripheral_varlist_tx(
                c, 1,
                sizeof(size_t), &rx_len);

        soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf, (uint16_t) rx_len);
    }

    soc_dma_ring_tx_buf_set(tx_ring_buf, tx_buf, (uint16_t) tx_len);

    /* this request will take care of both the RX and TX */
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);
}

void spi_master_device_init(
        soc_peripheral_t dev,
        unsigned cpol,
        unsigned cpha,
        unsigned clock_divide,
        unsigned cs_to_data_delay_ns,
        unsigned byte_setup_ns)
{
    chanend c_ctrl = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c_ctrl, SPI_MASTER_DEV_INIT);

    soc_peripheral_varlist_tx(
            c_ctrl, 5,
            sizeof(unsigned), &cpol,
            sizeof(unsigned), &cpha,
            sizeof(unsigned), &clock_divide,
            sizeof(unsigned), &cs_to_data_delay_ns,
            sizeof(unsigned), &byte_setup_ns);
}

void spi_transaction(
        soc_peripheral_t dev,
        uint8_t *rx_buf,
        size_t rx_len,
        uint8_t *tx_buf,
        size_t tx_len)
{
    SemaphoreHandle_t sem = (SemaphoreHandle_t) soc_peripheral_app_data(dev);

    if (tx_buf == NULL) {
        tx_len = 0;
    }

    if (rx_buf == NULL) {
        rx_len = 0;
    }

    spi_driver_transaction(dev,
                           rx_buf, rx_len,
                           tx_buf, tx_len);

    /* TODO: check return value */
    xSemaphoreTake(sem, portMAX_DELAY);

    if (rx_buf > 0) {
        /* TODO: check return value */
        xSemaphoreTake(sem, portMAX_DELAY);
    }
}
