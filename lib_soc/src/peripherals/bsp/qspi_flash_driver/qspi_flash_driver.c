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

typedef struct {
	soc_peripheral_t dev;
	SemaphoreHandle_t dma_sem;
	SemaphoreHandle_t lock;
} qspi_flash_driver_t;

RTOS_IRQ_ISR_ATTR
static void qspi_flash_isr(soc_peripheral_t dev)
{
	qspi_flash_driver_t *driver_struct = (qspi_flash_driver_t *) soc_peripheral_app_data(dev);
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
        		/*
        		 * TODO: Consider how not to give this semaphore for read operations.
        		 */
                ret = xSemaphoreGiveFromISR(driver_struct->dma_sem, &xYieldRequired);
                configASSERT(ret == pdTRUE);
        	}
        }
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

        while (soc_dma_ring_rx_buf_get(ring_buf, NULL, &more) != NULL) {
        	if (!more) {
                ret = xSemaphoreGiveFromISR(driver_struct->dma_sem, &xYieldRequired);
                configASSERT(ret == pdTRUE);
        	}
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
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
	qspi_flash_driver_t *driver_struct = (qspi_flash_driver_t *) soc_peripheral_app_data(dev);

	flash_cmd.operation = qspi_flash_dev_op_read;
	flash_cmd.byte_address = address;
	flash_cmd.byte_count = len;

	xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

	soc_dma_ring_rx_buf_set(rx_ring_buf, data, (uint16_t) len);
	soc_dma_ring_tx_buf_set(tx_ring_buf, &flash_cmd, (uint16_t) sizeof(flash_cmd));

    /* this request will take care of both the RX and TX */
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY); /* wait for the DMA TX to complete */
    xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY); /* wait for the DMA RX to complete */

    xSemaphoreGive(driver_struct->lock);
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
	qspi_flash_driver_t *driver_struct = (qspi_flash_driver_t *) soc_peripheral_app_data(dev);
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

	xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

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

    xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY); /* wait for the DMA TX to complete */

    xSemaphoreGive(driver_struct->lock);
}

void qspi_flash_erase(
        soc_peripheral_t dev,
		unsigned address,
        size_t len)
{
	qspi_flash_dev_cmd_t flash_cmd;
	soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);
	qspi_flash_driver_t *driver_struct = (qspi_flash_driver_t *) soc_peripheral_app_data(dev);

	flash_cmd.operation = qspi_flash_dev_op_erase;
	flash_cmd.byte_address = address;
	flash_cmd.byte_count = len;

	xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

	soc_dma_ring_tx_buf_set(tx_ring_buf, &flash_cmd, (uint16_t) sizeof(flash_cmd));

    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);

    /* wait for the DMA TX to complete before returning,
     * otherwise flash_cmd will go out of scope before
     * the device receives it. */
    xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY);

    xSemaphoreGive(driver_struct->lock);
}

soc_peripheral_t qspi_flash_driver_init(
        int device_id,
        int isr_core)
{
    soc_peripheral_t device;
    qspi_flash_driver_t *driver_struct;

    xassert(device_id >= 0 && device_id < BITSTREAM_QSPI_FLASH_DEVICE_COUNT);

    device = bitstream_qspi_flash_devices[device_id];

    driver_struct = pvPortMalloc(sizeof(qspi_flash_driver_t));

    driver_struct->dma_sem = xSemaphoreCreateCounting(2, 0);
    driver_struct->lock = xSemaphoreCreateMutex();
    driver_struct->dev = device;

    soc_peripheral_common_dma_init(
            device,
            1,
            0,
            3, /* Up to 3 TX buffers may be sent for a single transaction */
			driver_struct,
            isr_core,
            (rtos_irq_isr_t) qspi_flash_isr);

    return device;
}
