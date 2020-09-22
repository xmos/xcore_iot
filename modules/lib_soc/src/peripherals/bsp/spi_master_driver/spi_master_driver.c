// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "spi_master_driver.h"
#include "spi_master_dev_ctrl.h"
#include "soc_bsp_common.h"

#include "bitstream_devices.h"

#include "FreeRTOS.h"
#include "semphr.h"


#if ( SOC_SPI_PERIPHERAL_USED == 0 )
#define BITSTREAM_SPI_DEVICE_COUNT 0
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
#endif /* SOC_SPI_PERIPHERAL_USED */

typedef struct {
    soc_peripheral_t dev;
    SemaphoreHandle_t dma_sem;
    SemaphoreHandle_t lock;
    SPI_MASTER_ISR_CALLBACK_ATTR spi_master_isr_cb_t cb;
    uint32_t flags;
} spi_master_driver_t;

RTOS_IRQ_ISR_ATTR
static void spi_master_isr(soc_peripheral_t dev)
{
    spi_master_driver_t *driver_struct = (spi_master_driver_t *) soc_peripheral_app_data(dev);
    soc_dma_ring_buf_t *ring_buf;
    int more;
    BaseType_t xYieldRequired = pdFALSE;
    BaseType_t ret;
    uint32_t status;

    status = soc_peripheral_interrupt_status(dev);

    if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM) {
    	int i = 0;
    	int total_len = 0;
    	uint8_t *buf;
    	int buf_len;
        ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

        while ((buf = soc_dma_ring_tx_buf_get(ring_buf, &buf_len, &more)) != NULL) {
        	total_len += buf_len;
        	if (driver_struct->cb != NULL) {
        		driver_struct->cb(buf, buf_len, i, more, SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM, &xYieldRequired);
        	}
        	if (!more) {
        		i = 0;
        		if (!(driver_struct->flags & SPI_MASTER_FLAG_TX_NOBLOCK) && total_len > 0) {
					ret = xSemaphoreGiveFromISR(driver_struct->dma_sem, &xYieldRequired);
					configASSERT(ret == pdTRUE);
        		}
        	} else {
        		i++;
        	}
        }
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
    	int i = 0;
    	uint8_t *buf;
    	int buf_len;
        ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

        while ((buf = soc_dma_ring_rx_buf_get(ring_buf, &buf_len, &more)) != NULL) {
        	if (driver_struct->cb != NULL) {
        		driver_struct->cb(buf, buf_len, i, more, SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM, &xYieldRequired);
        	}
        	if (!more) {
        		i = 0;
        		if (!(driver_struct->flags & SPI_MASTER_FLAG_RX_NOBLOCK)) {
					ret = xSemaphoreGiveFromISR(driver_struct->dma_sem, &xYieldRequired);
					configASSERT(ret == pdTRUE);
        		}
        	} else {
        		i++;
        	}
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

static void spi_driver_transaction(
        soc_peripheral_t dev,
        uint8_t *rx_buf[],
        size_t rx_len[],
		size_t rx_buf_count,
		size_t total_rx_length,
        uint8_t *tx_buf[],
        size_t tx_len[],
		size_t tx_buf_count,
		size_t total_tx_length)
{
	int i;
    chanend c = soc_peripheral_ctrl_chanend(dev);
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(dev);

    if (total_rx_length > 0) {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

        soc_peripheral_function_code_tx(c, SPI_MASTER_DEV_TRANSACTION);

        soc_peripheral_varlist_tx(
                c, 1,
                sizeof(size_t), &total_rx_length);

        if (rx_buf_count == 1) {
        	soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf[0], (uint16_t) rx_len[0]);
        } else {
        	for (i = 1; i < rx_buf_count; i++) {
                soc_dma_ring_rx_buf_sg_set(
                        rx_ring_buf,
						rx_buf[i],
						rx_len[i],
                        i,
						rx_buf_count);
        	}
            soc_dma_ring_rx_buf_sg_set(
                    rx_ring_buf,
					rx_buf[0],
					rx_len[0],
                    0,
					rx_buf_count);
        }
    }

    if (tx_buf_count == 0) {
    	soc_dma_ring_tx_buf_set(tx_ring_buf, NULL, 0);
    } else if (tx_buf_count == 1) {
    	soc_dma_ring_tx_buf_set(tx_ring_buf, tx_buf[0], (uint16_t) tx_len[0]);
    } else {
    	for (i = 1; i < tx_buf_count; i++) {
            soc_dma_ring_tx_buf_sg_set(
                    tx_ring_buf,
					tx_buf[i],
					tx_len[i],
                    i,
					tx_buf_count);
    	}
        soc_dma_ring_tx_buf_sg_set(
                tx_ring_buf,
				tx_buf[0],
				tx_len[0],
                0,
				tx_buf_count);
    }

    /* this request will take care of both the RX and TX */
    soc_peripheral_hub_dma_request(dev, SOC_DMA_TX_REQUEST);
}

void spi_transaction_sg(
        soc_peripheral_t dev,
        uint8_t *rx_buf[],
        size_t rx_len[],
		size_t rx_buf_count,
        uint8_t *tx_buf[],
        size_t tx_len[],
		size_t tx_buf_count)
{
	spi_master_driver_t *driver_struct = (spi_master_driver_t *) soc_peripheral_app_data(dev);
	int i;

    if (tx_buf == NULL) {
    	tx_buf_count = 0;
    }

    if (rx_buf == NULL) {
    	rx_buf_count = 0;
    }

    size_t total_rx_length = 0;
    size_t total_tx_length = 0;

    for (i = 0; i < rx_buf_count; i++) {
    	total_rx_length += rx_len[i];
    }

    for (i = 0; i < tx_buf_count; i++) {
    	total_tx_length += tx_len[i];
    }

    xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

    spi_driver_transaction(dev,
                           rx_buf, rx_len, rx_buf_count, total_rx_length,
                           tx_buf, tx_len, tx_buf_count, total_tx_length);

    if (!(driver_struct->flags & SPI_MASTER_FLAG_TX_NOBLOCK) && total_tx_length > 0) {
    	/* TODO: check return value */
    	xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY);
    }

    if (!(driver_struct->flags & SPI_MASTER_FLAG_RX_NOBLOCK) && total_rx_length > 0) {
        /* TODO: check return value */
    	xSemaphoreTake(driver_struct->dma_sem, portMAX_DELAY);
    }

    xSemaphoreGive(driver_struct->lock);
}

void spi_transaction(
        soc_peripheral_t dev,
        uint8_t *rx_buf,
        size_t rx_len,
        uint8_t *tx_buf,
        size_t tx_len)
{
	spi_transaction_sg(
	        dev,
			rx_buf != NULL ? &rx_buf : NULL, &rx_len, 1,
			tx_buf != NULL ? &tx_buf : NULL, &tx_len, 1);
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

soc_peripheral_t spi_master_driver_init(
        int device_id,
        int dma_buffer_count,
        int isr_core,
		uint32_t flags,
		spi_master_isr_cb_t isr_cb)
{
    soc_peripheral_t device;
    spi_master_driver_t *driver_struct;

    xassert(device_id >= 0 && device_id < BITSTREAM_SPI_DEVICE_COUNT);

    /*
     * If either the TX or RX NOBLOCK flag is set, then
     * a callback ISR must be provided.
     */
    xassert((flags & (SPI_MASTER_FLAG_RX_NOBLOCK | SPI_MASTER_FLAG_TX_NOBLOCK)) == 0 || isr_cb != NULL);

    device = bitstream_spi_devices[device_id];

    driver_struct = pvPortMalloc(sizeof(spi_master_driver_t));

    if ((flags & (SPI_MASTER_FLAG_RX_NOBLOCK | SPI_MASTER_FLAG_TX_NOBLOCK)) == (SPI_MASTER_FLAG_RX_NOBLOCK | SPI_MASTER_FLAG_TX_NOBLOCK)) {
    	/* Do not need the semaphore if both noblock flags are set */
    	driver_struct->dma_sem = NULL;
    } else {
    	int max_count;

    	/*
    	 * Only one or zero noblock flags are set. If neither is set
    	 * then the ISR will increment the semaphore twice.
    	 * Otherwise, it will only increment it once.
    	 */

    	if ((flags & (SPI_MASTER_FLAG_RX_NOBLOCK | SPI_MASTER_FLAG_TX_NOBLOCK)) == 0) {
    		max_count = 2;
    	} else {
    		max_count = 1;
    	}

    	driver_struct->dma_sem = xSemaphoreCreateCounting(max_count, 0);
    }
    driver_struct->lock = xSemaphoreCreateMutex();
    driver_struct->flags = flags;
    driver_struct->cb = isr_cb;
    driver_struct->dev = device;

    soc_peripheral_common_dma_init(
            device,
            dma_buffer_count,
            0,
            dma_buffer_count,
			driver_struct,
            isr_core,
            (rtos_irq_isr_t) spi_master_isr);

    return device;
}
