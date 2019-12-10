// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "i2s_driver.h"

/* App headers */
#include "queue_to_i2s.h"

RTOS_IRQ_ISR_ATTR
int i2s_array_isr(soc_peripheral_t device)
{
    BaseType_t xYieldRequired = pdFALSE;
    return xYieldRequired;
}

void vqueue_to_i2s(void *arg)
{
    soc_peripheral_t i2s_dev = arg;
    QueueHandle_t input_queue = soc_peripheral_app_data(i2s_dev);
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(i2s_dev);
    int available = 4;

    for (;;) {
        int32_t *audio_data;
        int32_t *tx_buf;

        xQueueReceive(input_queue, &audio_data, portMAX_DELAY);

        while ((tx_buf = soc_dma_ring_tx_buf_get(tx_ring_buf, NULL, NULL)) != NULL || available < 2) {
            /*
             * Free the buffers sent to the I2S DAC after the DMA
             * engine sends them.
             */
            if (tx_buf != NULL) {
                vPortFree(tx_buf);
                available++;
            }
        }

        /*
         * Demonstrate the DMA scatter gather capability
         */

        char *audio_data2 = pvPortMalloc(sizeof(int32_t) * appconfMIC_FRAME_LENGTH / 2);
        memcpy(audio_data2, audio_data + appconfMIC_FRAME_LENGTH/2, sizeof(int32_t) * appconfMIC_FRAME_LENGTH/2);
        memset(audio_data + appconfMIC_FRAME_LENGTH/2, 0, sizeof(int32_t) * appconfMIC_FRAME_LENGTH/2);

        soc_dma_ring_tx_buf_sg_set(tx_ring_buf, audio_data2, sizeof(int32_t) * appconfMIC_FRAME_LENGTH/2,
                1, 2);
        soc_dma_ring_tx_buf_sg_set(tx_ring_buf, audio_data, sizeof(int32_t) * appconfMIC_FRAME_LENGTH/2,
                0, 2);

        available -= 2;

        soc_peripheral_hub_dma_request(i2s_dev, SOC_DMA_TX_REQUEST);
    }
}

void queue_to_i2s_create(QueueHandle_t input, UBaseType_t priority)
{
    soc_peripheral_t dev;

    dev = i2s_driver_init(
            BITSTREAM_I2S_DEVICE_A,             /* Initializing I2S device A */
            0,                                  /* Give this device no RX buffer descriptors */
            appconfMIC_FRAME_LENGTH * sizeof(int32_t), /* Make each DMA RX buffer MIC_FRAME_LENGTH samples */
            4,                                  /* Give this device 2 TX buffer descriptors */
            input,                              /* Queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) i2s_array_isr);    /* The ISR to handle this device's interrupts */

    xTaskCreate(vqueue_to_i2s, "queue_to_i2s", portTASK_STACK_DEPTH(vqueue_to_i2s), dev, priority, NULL);
}
