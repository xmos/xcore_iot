// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "audio_pipeline/audio_pipeline.h"
#include "example_pipeline.h"

#define GOOD 1

/*
 * Allow for 0.2 ms of overhead. This isn't needed when
 * the pipeline is run on the same core as the mics and i2s,
 * but when it is on the other core it is needed.
 */
#define OVERHEAD 20000

/*
 * Assumes 256 samples per frame, and 2 cores for 3 stages.
 */
#if GOOD

#if EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 16000
#define TIME_PER_STAGE (1060000 - OVERHEAD)
#elif EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 48000
#define TIME_PER_STAGE (350000 - OVERHEAD)
#else
#define TIME_PER_STAGE 0
#endif

#else /* not good */

#if EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 16000
#define TIME_PER_STAGE (1070000)
#elif EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE == 48000
#define TIME_PER_STAGE (360000 )
#else
#define TIME_PER_STAGE 0
#endif

#endif

#define TIME_PER_CYCLE (700.0 / 500.0 * 5.0 / 5.0)

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
	//rtos_printf("S0 on core %d\n", portGET_CORE_ID());

	for (int i = 0; i < EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH; i++)  {
		audio_frame[i][0] *= 2;
		audio_frame[i][1] *= 2;
	}

	uint32_t init_time = TIME_PER_STAGE * TIME_PER_CYCLE / 3;
	while (init_time-- > 0) asm volatile ("nop");
}

void stage1(int32_t (*audio_frame)[2])
{
	//rtos_printf("S1 on core %d\n", portGET_CORE_ID());

	for (int i = 0; i < EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH; i++)  {
        audio_frame[i][0] *= 3;
        audio_frame[i][1] *= 3;
	}

    uint32_t init_time = TIME_PER_STAGE * TIME_PER_CYCLE / 3;
    while (init_time-- > 0) asm volatile ("nop");
}

void stage2(int32_t (*audio_frame)[2])
{
	//rtos_printf("S2 on core %d\n", portGET_CORE_ID());

	for (int i = 0; i < EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH; i++)  {
        audio_frame[i][0] *= 4;
        audio_frame[i][1] *= 4;
	}

    uint32_t init_time = TIME_PER_STAGE * TIME_PER_CYCLE / 3;
    while (init_time-- > 0) asm volatile ("nop");
}

void example_pipeline_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx)
{
	const int stage_count = 3;

	const audio_pipeline_stage_t stages[stage_count] = {
			(audio_pipeline_stage_t) stage0,
			(audio_pipeline_stage_t) stage1,
			(audio_pipeline_stage_t) stage2,
	};

	const configSTACK_DEPTH_TYPE stage_stack_sizes[stage_count] = {
			configMINIMAL_STACK_SIZE,
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
