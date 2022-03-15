// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Library headers */
#include "generic_pipeline.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline/audio_pipeline.h"

typedef struct
{
    ap_ch_pair_t samples[AP_CHANNEL_PAIRS][AP_FRAME_ADVANCE];
    ap_ch_pair_t mic_samples_passthrough[AP_CHANNEL_PAIRS][AP_FRAME_ADVANCE];
} frame_data_t;

static void *audio_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));

    audio_pipeline_input(input_app_data,
                       (int32_t(*)[2])frame_data->samples,
                       AP_FRAME_ADVANCE);

    memcpy(frame_data->mic_samples_passthrough, frame_data->samples, sizeof(frame_data->mic_samples_passthrough));

    return frame_data;
}

static int audio_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    return audio_pipeline_output(output_app_data,
                               (int32_t(*)[2])frame_data->samples,
                               (int32_t(*)[2])frame_data->mic_samples_passthrough,
                               AP_FRAME_ADVANCE);
}

static void init_dsp_stage_0(void *state)
{
    /* Init dsp */
}

static void init_dsp_stage_1(void *state)
{
    /* Init dsp */
}

static void init_dsp_stage_2(void *state)
{
    /* Init dsp */
}

static void stage0(frame_data_t *frame_data)
{
    /* process frame here */

    memcpy(frame_data->samples,
           frame_data->samples,
           sizeof(frame_data->samples));
}

static void stage1(frame_data_t *frame_data)
{
    /* process frame here */

    memcpy(frame_data->samples,
           frame_data->samples,
           sizeof(frame_data->samples));
}

static void stage2(frame_data_t *frame_data)
{
    /* process frame here */

    /* Below is debug until real DSP blocks are integrated */
    /* Apply some gain so we can hear the mics with this skeleton app */
    for(int ch=0; ch<AP_CHANNEL_PAIRS; ch++) {
        for(int i=0; i<AP_FRAME_ADVANCE; i++) {
            frame_data->samples[ch][i].ch_a *= 256;
            frame_data->samples[ch][i].ch_b *= 256;
        }
    }
    /* End of debug */
}

void audio_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
    const int stage_count = 3;

    const pipeline_stage_t stages[] = {
        (pipeline_stage_t)stage0,
        (pipeline_stage_t)stage1,
        (pipeline_stage_t)stage2,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage0) + RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage1),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage2) + RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
    };

    init_dsp_stage_0(NULL);
    init_dsp_stage_1(NULL);
    init_dsp_stage_2(NULL);

    generic_pipeline_init((pipeline_input_t)audio_pipeline_input_i,
                        (pipeline_output_t)audio_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        appconfAUDIO_PIPELINE_TASK_PRIORITY,
                        stage_count);
}
