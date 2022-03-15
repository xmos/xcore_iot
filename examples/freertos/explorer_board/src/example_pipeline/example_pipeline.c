// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "generic_pipeline.h"
#include "example_pipeline/example_pipeline.h"
#include "platform/driver_instances.h"

#ifndef appconfPRINT_AUDIO_FRAME_POWER
#define appconfPRINT_AUDIO_FRAME_POWER 1
#endif

#if MIC_ARRAY_CONFIG_MIC_COUNT != 2
#error MIC_ARRAY_CONFIG_MIC_COUNT must be 2
#endif

#define Q31(f) (int)((signed long long)((f) * ((unsigned long long)1 << (31+20)) + (1<<19)) >> 20)
#define F31(x) ((double)(x)/(double)(uint32_t)(1<<31)) // needs uint32_t cast because bit 31 is 1

#undef MIN
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

void frame_power(int32_t (*audio_frame)[MIC_ARRAY_CONFIG_MIC_COUNT])
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

    rtos_printf("Stage 1 mic power:\nch0: %d\nch1: %d\n",
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))),
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))));
}

void *example_pipeline_input(void *data)
{
    (void) data;

    int32_t (*audio_frame)[MIC_ARRAY_CONFIG_MIC_COUNT];

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

    rtos_i2s_tx(
            i2s_ctx,
            audio_frame,
            appconfAUDIO_FRAME_LENGTH,
            portMAX_DELAY);

    vPortFree(audio_frame);

    return 0;
}

void stage0(int32_t (*audio_frame)[MIC_ARRAY_CONFIG_MIC_COUNT])
{
#if appconfPRINT_AUDIO_FRAME_POWER
    frame_power(audio_frame);
#endif
}

void stage1(int32_t (*audio_frame)[MIC_ARRAY_CONFIG_MIC_COUNT])
{
	for (int i = 0; i < appconfAUDIO_FRAME_LENGTH; i++)  {
        audio_frame[i][0] *= xStage1_Gain;
        audio_frame[i][1] *= xStage1_Gain;
	}
}

void example_pipeline_init(UBaseType_t priority)
{
	const int stage_count = 2;

	const pipeline_stage_t stages[stage_count] = {
			(pipeline_stage_t) stage0,
			(pipeline_stage_t) stage1
	};

	const configSTACK_DEPTH_TYPE stage_stack_sizes[stage_count] = {
			configMINIMAL_STACK_SIZE,
			configMINIMAL_STACK_SIZE
	};

	generic_pipeline_init(
			example_pipeline_input,
			example_pipeline_output,
            NULL,
            NULL,
			stages,
			(const size_t*) stage_stack_sizes,
			priority,
			stage_count);
}

#undef MIN
