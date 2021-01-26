// Copyright (c) 2020-2021, XMOS Ltd, All rights reserved

#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "app_conf.h"
#include "dsp_qformat.h"

#include "audio_pipeline/audio_pipeline.h"
#include "example_pipeline/example_pipeline.h"


#ifndef appconfPRINT_AUDIO_FRAME_POWER
#define appconfPRINT_AUDIO_FRAME_POWER 0
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


#define TIME_PER_CYCLE (700.0 / 500.0 * 5.0 / 5.0)

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

void frame_power(int32_t (*audio_frame)[2])
{
    uint64_t frame_power0 = 0;
    uint64_t frame_power1 = 0;

    for (int i = 0; i < EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH; ++i) {
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
    rtos_mic_array_t *mic_array_ctx = data;

    int32_t (*audio_frame)[2];

    audio_frame = pvPortMalloc(EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH * sizeof(audio_frame[0]));

    rtos_mic_array_rx(
            mic_array_ctx,
            audio_frame,
            EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH,
            portMAX_DELAY);

    return audio_frame;
}

int example_pipeline_output(void *audio_frame, void *data)
{
    rtos_i2s_master_t *i2s_master_ctx = data;

    rtos_i2s_master_tx(
            i2s_master_ctx,
            audio_frame,
            EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH,
            portMAX_DELAY);

    vPortFree(audio_frame);

    return 0;
}

void stage0(int32_t (*audio_frame)[2])
{
#if appconfPRINT_AUDIO_FRAME_POWER
    frame_power(audio_frame);
#endif
}

void stage1(int32_t (*audio_frame)[2])
{
	for (int i = 0; i < EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH; i++)  {
        audio_frame[i][0] *= xStage1_Gain;
        audio_frame[i][1] *= xStage1_Gain;
	}
}

void example_pipeline_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx)
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
			mic_array_ctx,
			i2s_master_ctx,
			stages,
			stage_stack_sizes,
			configMAX_PRIORITIES / 2,
			stage_count);
}
