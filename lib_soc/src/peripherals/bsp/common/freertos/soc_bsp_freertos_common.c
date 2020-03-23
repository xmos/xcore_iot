// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"

#if RTOS_FREERTOS

#include "FreeRTOS.h"
#include "semphr.h"

void soc_peripheral_common_dma_init(
        soc_peripheral_t device,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_dma_ring_buf_t *ring_buf;
    uint32_t *buf_desc;
    int i;

#ifdef taskVALID_CORE_ID
    configASSERT(taskVALID_CORE_ID(isr_core));
#endif

    if (rx_desc_count > 0) {
        buf_desc = pvPortMalloc(rx_desc_count * SOC_DMA_BUF_DESC_WORDSIZE * sizeof(uint32_t));
        configASSERT(buf_desc != NULL);

        ring_buf = soc_peripheral_rx_dma_ring_buf(device);
        soc_dma_ring_buf_init(ring_buf, buf_desc, rx_desc_count);

        if (rx_buf_size > 0) {
            for (i = 0; i < rx_desc_count; i++) {
                void *buf = pvPortMalloc(rx_buf_size);
                configASSERT(buf != NULL);
                soc_dma_ring_rx_buf_set(ring_buf, buf, rx_buf_size);
            }
        }
    }

    if (tx_desc_count > 0) {
        buf_desc = pvPortMalloc(tx_desc_count * SOC_DMA_BUF_DESC_WORDSIZE * sizeof(uint32_t));
        configASSERT(buf_desc != NULL);
        ring_buf = soc_peripheral_tx_dma_ring_buf(device);
        soc_dma_ring_buf_init(ring_buf, buf_desc, tx_desc_count);
        ring_buf->tx_semaphore = xSemaphoreCreateCounting(tx_desc_count, tx_desc_count);
    }

    soc_peripheral_handler_register(device, isr_core, app_data, isr);
}

#endif /* RTOS_FREERTOS */

