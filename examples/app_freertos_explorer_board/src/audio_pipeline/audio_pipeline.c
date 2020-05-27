// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include <string.h>
#include "soc.h"
#include "dsp_qformat.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "micarray_driver.h"
#include "i2c_driver.h"
#include "i2s_driver.h"

/* App headers */
#include "audio_pipeline.h"
#include "app_conf.h"
#include "audio_hw_config.h"
#include "queue_to_tcp_stream.h"
#include "tcp_stream_to_queue.h"
#include "queue_to_i2s.h"

static BaseType_t xStage1_Gain = appconfAUDIO_PIPELINE_STAGE_ONE_GAIN;
static state2_input_sel_t input_sel = eAPINPUT_QUEUE;

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

BaseType_t audiopipeline_get_stage2_input( void )
{
    return input_sel;
}

BaseType_t audiopipeline_set_stage2_input( state2_input_sel_t input )
{
	input_sel = input;
    return input_sel;
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
void mic_array_isr(soc_peripheral_t device)
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
        rx_buf = soc_dma_ring_rx_buf_get(rx_ring_buf, &length, NULL);
        configASSERT(rx_buf != NULL);
//        debug_printf("mic data rx %d bytes\n", length);

        if (xQueueSendFromISR(mic_data_queue, &rx_buf, &xYieldRequired) == errQUEUE_FULL) {
//            debug_printf("mic data lost\n", length);
            soc_dma_ring_rx_buf_set(rx_ring_buf, rx_buf, sizeof(int32_t) * appconfMIC_FRAME_LENGTH);
            soc_peripheral_hub_dma_request(device, SOC_DMA_RX_REQUEST);
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

/* Apply gain to mic data */
void audio_pipeline_stage1(void *arg)
{
	ap_stage_handle_t stage1 = ( ap_stage_handle_t ) arg;
    soc_peripheral_t mic_dev = ( soc_peripheral_t ) stage1->args;
    soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(mic_dev);

    for (;;) {
        int32_t *mic_data;
        int32_t *new_rx_buffer;

        xQueueReceive(stage1->input, &mic_data, portMAX_DELAY);

        new_rx_buffer = pvPortMalloc(appconfMIC_FRAME_LENGTH * sizeof(int32_t));
        soc_dma_ring_rx_buf_set(rx_ring_buf, new_rx_buffer, sizeof(int32_t) * appconfMIC_FRAME_LENGTH);
        soc_peripheral_hub_dma_request(mic_dev, SOC_DMA_RX_REQUEST);

#if appconfPRINT_AUDIO_FRAME_POWER
        debug_printf("Mic power: %d\n", frame_power(mic_data));
#endif

        for (int i = 0; i < appconfMIC_FRAME_LENGTH; i++) {
            mic_data[i] *= xStage1_Gain;
        }

        if (xQueueSend(stage1->output, &mic_data, pdMS_TO_TICKS(1)) == errQUEUE_FULL)
        {
//            debug_printf("stage1 output lost\n");
            vPortFree(mic_data);
        }
    }
}

/* Output to tcp and i2s */
void audio_pipeline_stage2(void *arg)
{
	ap_stage_handle_t stage2 = ( ap_stage_handle_t ) arg;
    BaseType_t recv_data = pdFALSE;
    int32_t *mic_data;

	QueueHandle_t stage2_to_tcp_queue = xQueueCreate(2, sizeof(void *));

    /* Create queue to tcp task */
    queue_to_tcp_handle_t mic_to_tcp_handle;
    mic_to_tcp_handle = queue_to_tcp_create( stage2_to_tcp_queue,
											 appconfQUEUE_TO_TCP_PORT,
											 portMAX_DELAY,
											 pdMS_TO_TICKS( 5000 ),
											 sizeof(int32_t) * appconfMIC_FRAME_LENGTH );
    queue_to_tcp_stream_create( mic_to_tcp_handle, appconfQUEUE_TO_TCP_TASK_PRIORITY );

    QueueHandle_t tcp2q = xQueueCreate(2, sizeof(void *));
    tcp_to_queue_handle_t handle = tcp_to_queue_create(
    										tcp2q,
											appconfTCP_TO_QUEUE_PORT,
											portMAX_DELAY,
											portMAX_DELAY,
											sizeof(int32_t)*appconfMIC_FRAME_LENGTH );
    tcp_stream_to_queue_create( handle, appconfTCP_TO_QUEUE_TASK_PRIORITY );

    for (;;) {
    	/* Attempt to receive and item from an input queue.
    	 * Timeout after 50ms, in case the input queue was changed. */
        switch( input_sel )
        {
			case eAPINPUT_QUEUE:
				recv_data = xQueueReceive(stage2->input, &mic_data, pdMS_TO_TICKS(50) );
				break;
			case eTCP_QUEUE:
				recv_data = xQueueReceive(tcp2q, &mic_data, pdMS_TO_TICKS(50));
				break;
			default:
				debug_printf("Invalid stage2 input select\n");
				break;
        }

        if( recv_data == pdTRUE )
        {
			/* If queue_to_tcp is connected, make a copy of the data and give it to the queue*/
			if( is_queue_to_tcp_connected( mic_to_tcp_handle ) )
			{
				int32_t *mic_data_copy;
				mic_data_copy = pvPortMalloc(appconfMIC_FRAME_LENGTH * sizeof(int32_t));
				memcpy(mic_data_copy, mic_data, appconfMIC_FRAME_LENGTH * sizeof(int32_t));

				if (xQueueSend(stage2_to_tcp_queue, &mic_data_copy, pdMS_TO_TICKS(1)) == errQUEUE_FULL) {
//					debug_printf("stage2 mic to tcp output lost\n");
					vPortFree(mic_data_copy);
				}
			}
			if( stage2->output != NULL )
			{
				if (xQueueSend(stage2->output, &mic_data, pdMS_TO_TICKS(1)) == errQUEUE_FULL)
				{
//         			debug_printf("stage2 dac output lost\n");
					vPortFree(mic_data);
				}
			}
			else
			{
				vPortFree(mic_data);
			}
        }
    }
}

void audio_pipeline_create( UBaseType_t priority )
{
    QueueHandle_t mic_dev_queue;
    QueueHandle_t stage1_out_queue;
    QueueHandle_t stage2_out_queue;
    soc_peripheral_t dev;

    mic_dev_queue = xQueueCreate(2, sizeof(void *));
    stage1_out_queue = xQueueCreate(2, sizeof(void *));
    stage2_out_queue = xQueueCreate(2, sizeof(void *));

    /*
     * Configure the PLL and DACs. Do this now before
     * starting the scheduler.
     */
    dev = i2c_driver_init(BITSTREAM_I2C_DEVICE_A);
    audio_hw_config(dev);

    dev = micarray_driver_init(
            BITSTREAM_MICARRAY_DEVICE_A,       /* Initializing mic array device A */
            3,                                  /* Give this device 3 RX buffer descriptors */
            appconfMIC_FRAME_LENGTH * sizeof(int32_t), /* Make each DMA RX buffer MIC_FRAME_LENGTH samples */
            0,                                  /* Give this device no TX buffer descriptors */
			mic_dev_queue,                              /* The queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) mic_array_isr); /* The ISR to handle this device's interrupts */

    ap_stage_handle_t stage1 = pvPortMalloc( sizeof( ap_stage_t ) );
    stage1->args = dev;
    stage1->input = mic_dev_queue;
    stage1->output = stage1_out_queue;

    xTaskCreate(audio_pipeline_stage1, "stage1", portTASK_STACK_DEPTH(audio_pipeline_stage1), stage1, priority, NULL);

    ap_stage_handle_t stage2 = pvPortMalloc( sizeof( ap_stage_t ) );
    stage2->args = NULL;
    stage2->input = stage1_out_queue;
    stage2->output = stage2_out_queue;
    xTaskCreate(audio_pipeline_stage2, "stage2", portTASK_STACK_DEPTH(audio_pipeline_stage2), stage2, priority, NULL);

    /* Create queue to i2s task */
    queue_to_i2s_create( stage2_out_queue, appconfQUEUE_TO_I2S_TASK_PRIORITY );
}
