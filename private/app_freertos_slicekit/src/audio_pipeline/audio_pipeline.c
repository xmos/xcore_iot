// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOS_IP.h"

/* Library headers */
#include <string.h>
#include "soc.h"
#include "dsp_qformat.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "micarray_driver.h"

/* App headers */
#include "audio_pipeline.h"
#include "app_conf.h"
#include "queue_to_tcp_stream.h"

static BaseType_t xStage1_Gain = appconfAUDIO_PIPELINE_STAGE_ONE_GAIN;

static QueueHandle_t stage1_out_queue0;

#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

BaseType_t audiopipeline_get_stage1_gain( void )
{
    return xStage1_Gain;
}

BaseType_t audiopipeline_set_stage1_gain( BaseType_t xnewgain )
{
    xStage1_Gain = xnewgain;
    return xStage1_Gain;
}

int frame_power(int32_t *mic_data)
{
    uint64_t frame_power = 0;
    int return_power;

    for (size_t k = 0; k < appconfMIC_FRAME_LENGTH; ++k) {
        int64_t smp = mic_data[k];
        frame_power += (smp * smp) >> 31;
    }

    /* divide by appconfMIC_FRAME_LENGTH (2^8) */
    frame_power >>= 8;

    return_power = MIN(frame_power, (uint64_t) Q31(F31(INT32_MAX)));

    return return_power;
}

RTOS_IRQ_ISR_ATTR
int mic_array_isr(soc_peripheral_t device)
{
    QueueHandle_t mic_data_queue = soc_peripheral_app_data(device);
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    status = soc_peripheral_interrupt_status(device);

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM) {
        soc_dma_ring_buf_t *rx_ring_buf;
        int length;
        int32_t *rx_buf;

        /*
         * This must be from the mic array device
         */
        configASSERT(device == bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A]);

        rx_ring_buf = soc_peripheral_rx_dma_ring_buf(device);
        rx_buf = soc_dma_ring_rx_buf_get(rx_ring_buf, &length);
        configASSERT(rx_buf != NULL);
//        debug_printf("mic data rx %d bytes\n", length);

        if (xQueueSendFromISR(mic_data_queue, &rx_buf, &xYieldRequired) == errQUEUE_FULL) {
//            debug_printf("mic data lost\n", length);
            soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf, sizeof(int32_t) * appconfMIC_FRAME_LENGTH);
            soc_peripheral_hub_dma_request(device, SOC_DMA_RX_REQUEST);
        }
    }

    return xYieldRequired;
}

/* Apply gain to mic data */
void audio_pipeline_stage1(void *arg)
{
    soc_peripheral_t mic_dev = arg;
    QueueHandle_t mic_data_queue = soc_peripheral_app_data(mic_dev);
    soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(mic_dev);

    for (;;) {
        int32_t *mic_data;
        int32_t *new_rx_buffer;

        xQueueReceive(mic_data_queue, &mic_data, portMAX_DELAY);

        new_rx_buffer = pvPortMalloc(appconfMIC_FRAME_LENGTH * sizeof(int32_t));
        soc_dma_ring_rx_buf_set(rx_ring_buf, new_rx_buffer, sizeof(int32_t) * appconfMIC_FRAME_LENGTH);
        soc_peripheral_hub_dma_request(mic_dev, SOC_DMA_RX_REQUEST);

        //debug_printf("Mic power: %d\n", frame_power(mic_data));

        for (int i = 0; i < appconfMIC_FRAME_LENGTH; i++) {
            mic_data[i] *= xStage1_Gain;
        }

        if ( is_queue_to_tcp_connected() )
        {
            if (xQueueSend(stage1_out_queue0, &mic_data, pdMS_TO_TICKS(1)) == errQUEUE_FULL) {
                //            debug_printf("stage 1 output lost\n");
                vPortFree(mic_data);
            }
        }
        else
        {
            vPortFree(mic_data);
        }

    }
}

void audio_pipeline_create(QueueHandle_t output, UBaseType_t priority)
{
    QueueHandle_t queue;
    soc_peripheral_t dev;

    stage1_out_queue0 = output;

    queue = xQueueCreate(2, sizeof(void *));
    dev = micarray_driver_init(
            BITSTREAM_MICARRAY_DEVICE_A,       /* Initializing mic array device A */
            3,                                  /* Give this device 3 RX buffer descriptors */
            appconfMIC_FRAME_LENGTH * sizeof(int32_t), /* Make each DMA RX buffer MIC_FRAME_LENGTH samples */
            0,                                  /* Give this device no TX buffer descriptors */
            queue,                              /* The queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) mic_array_isr); /* The ISR to handle this device's interrupts */

    xTaskCreate(audio_pipeline_stage1, "stage1", portTASK_STACK_DEPTH(audio_pipeline_stage1), dev, priority, NULL);
}
