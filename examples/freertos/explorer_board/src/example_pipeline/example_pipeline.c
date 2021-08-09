// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Library headers */
#include "dsp_qformat.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline/audio_pipeline.h"
#include "example_pipeline/example_pipeline.h"
#include "queue_to_tcp_stream/queue_to_tcp_stream.h"
#include "platform/driver_instances.h"

#define FRAME_NUM_CHANS 2

#ifndef appconfPRINT_AUDIO_FRAME_POWER
#define appconfPRINT_AUDIO_FRAME_POWER 1
#endif

/*
 * Allow for 0.2 ms of overhead. This isn't needed when
 * the pipeline is run on the same core as the mics and i2s,
 * but when it is on the other core it is needed.
 */
#define OVERHEAD 20000

/*
 * Assumes 256 samples per frame, and 2 cores for 3 stages.
 */
#if EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 16000
#define TIME_PER_STAGE (1060000 - OVERHEAD)
#elif EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 48000
#define TIME_PER_STAGE (350000 - OVERHEAD)
#else
#define TIME_PER_STAGE 0
#endif

#if FRAME_NUM_CHANS != 2
#error FRAME_NUM_CHANS must be 2
#endif


#define TIME_PER_CYCLE (600.0 / 500.0 * 5.0 / 5.0)

#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

static BaseType_t xStage1_Gain = appconfAUDIO_PIPELINE_STAGE_ONE_GAIN;

BaseType_t audiopipeline_get_stage1_gain( void )
{
    rtos_printf("Gain currently is: %d\n", xStage1_Gain);
    return xStage1_Gain;
}

BaseType_t audiopipeline_set_stage1_gain( BaseType_t xNewGain )
{
    xStage1_Gain = xNewGain;
    rtos_printf("Gain currently is: %d new gain is\n", xStage1_Gain, xNewGain);
    return xStage1_Gain;
}

void frame_power(int32_t (*audio_frame)[FRAME_NUM_CHANS])
{
    uint64_t frame_power0 = 0;
    uint64_t frame_power1 = 0;

    for (int i = 0; i < appconfAUDIO_FRAME_LENGTH; ++i) {
        int64_t smp = audio_frame[i][0];
        frame_power0 += (smp * smp) >> 31;
        smp = audio_frame[i][1];
        frame_power1 += (smp * smp) >> 31;
    }

    /* divide by appconfMIC_FRAME_LENGTH (2^8) */
    frame_power0 >>= 8;
    frame_power1 >>= 8;

    debug_printf("Stage 1 mic power:\nch0: %d\nch1: %d\n",
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))),
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))));
}

void *example_pipeline_input(void *data)
{
    (void) data;

    int32_t (*audio_frame)[FRAME_NUM_CHANS];

    audio_frame = pvPortMalloc(appconfAUDIO_FRAME_LENGTH * sizeof(audio_frame[0]));

    rtos_mic_array_rx(
            mic_array_ctx,
            audio_frame,
            appconfAUDIO_FRAME_LENGTH,
            portMAX_DELAY);

    return audio_frame;
}

int example_pipeline_output(void *audio_frame, void *data)
{
    (void) data;

    rtos_intertile_tx(
            intertile_ctx,
            appconfINTERTILE_AUDIOPIPELINE_PORT,
            audio_frame,
            FRAME_NUM_CHANS * appconfAUDIO_FRAME_LENGTH * sizeof(int32_t));

    rtos_i2s_tx(
            i2s_ctx,
            audio_frame,
            appconfAUDIO_FRAME_LENGTH,
            portMAX_DELAY);

    vPortFree(audio_frame);

    return 0;
}

void stage0(int32_t (*audio_frame)[FRAME_NUM_CHANS])
{
#if appconfPRINT_AUDIO_FRAME_POWER
    frame_power(audio_frame);
#endif
}

void stage1(int32_t (*audio_frame)[FRAME_NUM_CHANS])
{
	for (int i = 0; i < appconfAUDIO_FRAME_LENGTH; i++)  {
        audio_frame[i][0] *= xStage1_Gain;
        audio_frame[i][1] *= xStage1_Gain;
	}
}

static void intertile_audiopipeline_thread(QueueHandle_t output_queue)
{
    int msg_length;
    int32_t *msg;
    int32_t *data = NULL;

    for (;;) {
        msg_length = rtos_intertile_rx(
                            intertile_ctx,
                            appconfINTERTILE_AUDIOPIPELINE_PORT,
                            (void **) &msg,
                            portMAX_DELAY);

        configASSERT(msg_length == FRAME_NUM_CHANS * appconfAUDIO_FRAME_LENGTH * sizeof(int32_t));

        data = pvPortMalloc(sizeof(int32_t) * appconfAUDIO_FRAME_LENGTH);

        for (int i = 0; i < appconfAUDIO_FRAME_LENGTH; ++i) {
            data[i] = msg[i*2];
        }

        vPortFree(msg);

        if (xQueueSend(output_queue, &data, pdMS_TO_TICKS(1)) == errQUEUE_FULL)
        {
            // rtos_printf("intertile rx mic frame lost\n");
            vPortFree(data);
        }
    }
}

static void intertile_pipeline_client_init(
    QueueHandle_t output_queue)
{
    xTaskCreate((TaskFunction_t) intertile_audiopipeline_thread,
                "intertile_ap_thread",
                RTOS_THREAD_STACK_SIZE(intertile_audiopipeline_thread),
                output_queue,
                appconfINTERTILE_AUDIOPIPELINE_TASK_PRIORITY,
                NULL);
}

void intertile_pipeline_to_tcp_create(void)
{
    QueueHandle_t output_queue = xQueueCreate(2, sizeof(void *));
    if( output_queue != NULL )
    {
        queue_to_tcp_handle_t mic_to_tcp_handle = queue_to_tcp_create(
                output_queue,
                appconfQUEUE_TO_TCP_PORT,
                portMAX_DELAY,
                pdMS_TO_TICKS( 5000 ),
                sizeof(int32_t) * appconfAUDIO_FRAME_LENGTH );

        intertile_pipeline_client_init(
                mic_to_tcp_handle->queue);
        queue_to_tcp_stream_create(
                mic_to_tcp_handle,
                (appconfINTERTILE_AUDIOPIPELINE_TASK_PRIORITY + 1) );
    }
}

void example_pipeline_init(UBaseType_t priority)
{
	const int stage_count = 2;

	const audio_pipeline_stage_t stages[stage_count] = {
			(audio_pipeline_stage_t) stage0,
			(audio_pipeline_stage_t) stage1
	};

	const configSTACK_DEPTH_TYPE stage_stack_sizes[stage_count] = {
			configMINIMAL_STACK_SIZE,
			configMINIMAL_STACK_SIZE
	};

	audio_pipeline_init(
			example_pipeline_input,
			example_pipeline_output,
            NULL,
            NULL,
			stages,
			stage_stack_sizes,
			priority,
			stage_count);
}
