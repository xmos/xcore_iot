// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* std headers */
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* App headers */
#include "audio_pipeline/audio_pipeline.h"
#include "example_pipeline/example_pipeline.h"

typedef struct {
    int32_t ch_a;
    int32_t ch_b;
} ch_pair_t;

typedef struct
{
    ch_pair_t samples[EXAMPLE_PIPELINE_CHANNEL_PAIRS][EXAMPLE_PIPELINE_FRAME_ADVANCE];
    ch_pair_t mic_samples_passthrough[EXAMPLE_PIPELINE_CHANNEL_PAIRS][EXAMPLE_PIPELINE_FRAME_ADVANCE];
} frame_data_t;

static void *vfe_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));

    vfe_pipeline_input(input_app_data,
                       (int32_t(*)[2])frame_data->samples,
                       EXAMPLE_PIPELINE_FRAME_ADVANCE);

    memcpy(frame_data->mic_samples_passthrough, frame_data->samples, sizeof(frame_data->mic_samples_passthrough));

    return frame_data;
}

static int vfe_pipeline_output_i(frame_data_t *frame_data,
                                 void *output_app_data)
{
    return vfe_pipeline_output(output_app_data,
                               (int32_t(*)[2])frame_data->samples,
                               (int32_t(*)[2])frame_data->mic_samples_passthrough,
                               EXAMPLE_PIPELINE_FRAME_ADVANCE);
}

// static void init_dsp_stage_0(vfe_dsp_stage_0_state_t *state)
// {
// }

static void stage0(frame_data_t *frame_data)
{
    memcpy(frame_data->samples,
           frame_data->samples,
           sizeof(frame_data->samples));
}

static void stage1(frame_data_t *frame_data)
{
    memcpy(frame_data->samples,
           frame_data->samples,
           sizeof(frame_data->samples));
}

static void stage2(frame_data_t *frame_data)
{
    /* Apply some gain so we can hear the mics with this skeleton app */
    for(int ch=0; ch<EXAMPLE_PIPELINE_CHANNEL_PAIRS; ch++) {
        for(int i=0; i<EXAMPLE_PIPELINE_FRAME_ADVANCE; i++) {
            frame_data->samples[ch][i].ch_a *= 256;
            frame_data->samples[ch][i].ch_b *= 256;
        }
    }
}

void vfe_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
    const int stage_count = 3;

    const audio_pipeline_stage_t stages[] = {
        (audio_pipeline_stage_t)stage0,
        (audio_pipeline_stage_t)stage1,
        (audio_pipeline_stage_t)stage2,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage0) + RTOS_THREAD_STACK_SIZE(vfe_pipeline_input_i),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage1),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage2) + RTOS_THREAD_STACK_SIZE(vfe_pipeline_output_i),
    };

    // init_dsp_stage_0(&dsp_stage_0_state);

    audio_pipeline_init((audio_pipeline_input_t)vfe_pipeline_input_i,
                        (audio_pipeline_output_t)vfe_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        stage_stack_sizes,
                        configMAX_PRIORITIES / 2,
                        stage_count);
}
