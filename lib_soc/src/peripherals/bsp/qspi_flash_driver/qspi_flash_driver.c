// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "qspi_flash_driver.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if ( SOC_QSPI_FLASH_PERIPHERAL_USED == 0 )
#define BITSTREAM_QSPI_FLASH_DEVICE_COUNT 0
soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];
#endif /* SOC_QSPI_FLASH_PERIPHERAL_USED */

RTOS_IRQ_ISR_ATTR
static void qspi_flash_isr(soc_peripheral_t dev)
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

        while (soc_dma_ring_tx_buf_get(ring_buf, NULL, &more) != NULL) {
        	if (!more) {
        		rtos_printf("qspi tx isr\n");
                ret = xSemaphoreGiveFromISR(sem, &xYieldRequired);
                configASSERT(ret == pdTRUE);
        	}
        }
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

        while (soc_dma_ring_rx_buf_get(ring_buf, NULL, &more) != NULL) {
        	if (!more) {
        		rtos_printf("qspi rx isr\n");
                ret = xSemaphoreGiveFromISR(sem, &xYieldRequired);
                configASSERT(ret == pdTRUE);
        	}
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

soc_peripheral_t qspi_flash_driver_init(
        int device_id,
        int isr_core)
{
    soc_peripheral_t device;

    SemaphoreHandle_t sem;
    sem = xSemaphoreCreateCounting(2, 0);

    xassert(device_id >= 0 && device_id < BITSTREAM_QSPI_FLASH_DEVICE_COUNT);

    device = bitstream_qspi_flash_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            1,
            0,
            3, /* Up to 3 TX buffers may be sent for a single transaction */
            sem,
            isr_core,
            (rtos_irq_isr_t) qspi_flash_isr);

    return device;
}

void qspi_flash_read(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len)
{
	qspi_flash_dev_cmd_t flash_cmd;
	soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
	soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);
	SemaphoreHandle_t sem = (SemaphoreHandle_t) soc_peripheral_app_data(dev);

	flash_cmd.operation = qspi_flash_dev_op_read;
	flash_cmd.byte_address = address;
	flash_cmd.byte_count = len;

	soc_dma_ring_rx_buf_set(rx_ring_buf, data, (uint16_t) len);
	soc_dma_ring_tx_buf_set(tx_ring_buf, &flash_cmd, (uint16_t) sizeof(flash_cmd));

    /* this request will take care of both the RX and TX */
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    xSemaphoreTake(sem, portMAX_DELAY); /* wait for the DMA TX to complete */
    xSemaphoreTake(sem, portMAX_DELAY); /* wait for the DMA RX to complete */
    rtos_printf("read complete\n");
}

#define WORD_TO_BYTE_ADDRESS(w) ((w) * sizeof(uint32_t))
#define BYTE_TO_WORD_ADDRESS(b) ((b) / sizeof(uint32_t))

void qspi_flash_write(
        soc_peripheral_t dev,
        uint8_t *data,
		unsigned address,
        size_t len)
{
	qspi_flash_dev_cmd_t flash_cmd;
	soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
	SemaphoreHandle_t sem = (SemaphoreHandle_t) soc_peripheral_app_data(dev);
#if QSPI_FLASH_QUAD_PAGE_PROGRAM
	/*
	 * If the address is not word aligned, then the start
	 * of the data sent to the QSPI flash device will be
	 * padded.
	 */
	uint8_t padding[sizeof(uint32_t) - 1] = {0xFF, 0xFF, 0xFF};
	int pad_len = address - WORD_TO_BYTE_ADDRESS(BYTE_TO_WORD_ADDRESS(address));
#endif
	/*
	 * At least two tx buffers will be sent in a single transaction.
	 * The first will have the flash command.
	 * Optionally the next one will have padding bytes.
	 * The last will have the data to write to flash.
	 */
	int tx_buffers = 2;
	int tx_buf_index = 1;

	flash_cmd.operation = qspi_flash_dev_op_write;
	flash_cmd.byte_address = address;
	flash_cmd.byte_count = len;

#if QUAD_PAGE_PROGRAM
	if (pad_len > 0) {
		tx_buffers++;
		soc_dma_ring_tx_buf_sg_set(tx_ring_buf, padding, pad_len, tx_buf_index, tx_buffers);
		tx_buf_index++;
	}
#endif
	soc_dma_ring_tx_buf_sg_set(tx_ring_buf, data, (uint16_t) len, tx_buf_index, tx_buffers);
	soc_dma_ring_tx_buf_sg_set(tx_ring_buf, &flash_cmd, (uint16_t) sizeof(flash_cmd), 0, tx_buffers);

    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    xSemaphoreTake(sem, portMAX_DELAY); /* wait for the DMA TX to complete */
    rtos_printf("write complete\n");
}

void qspi_flash_erase(
        soc_peripheral_t dev,
		unsigned address,
        size_t len)
{
	qspi_flash_dev_cmd_t flash_cmd;
	soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
	SemaphoreHandle_t sem = (SemaphoreHandle_t) soc_peripheral_app_data(dev);

	flash_cmd.operation = qspi_flash_dev_op_erase;
	flash_cmd.byte_address = address;
	flash_cmd.byte_count = len;

	soc_dma_ring_tx_buf_set(tx_ring_buf, &flash_cmd, (uint16_t) sizeof(flash_cmd));

    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    xSemaphoreTake(sem, portMAX_DELAY); /* wait for the DMA TX to complete */
    rtos_printf("erase complete\n");
}


#if 0
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
        unsigned byte_setup_ns,
        unsigned data_to_cs_delay_ns)
{
    chanend c_ctrl = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c_ctrl, SPI_MASTER_DEV_INIT);

    soc_peripheral_varlist_tx(
            c_ctrl, 6,
            sizeof(unsigned), &cpol,
            sizeof(unsigned), &cpha,
            sizeof(unsigned), &clock_divide,
            sizeof(unsigned), &cs_to_data_delay_ns,
            sizeof(unsigned), &byte_setup_ns,
            sizeof(unsigned), &data_to_cs_delay_ns);
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
#endif
