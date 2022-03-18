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
#include "agc_api.h"
#include "ic_api.h"
#include "ns_api.h"
#include "vad_api.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline/audio_pipeline.h"

/* Note: Changing the order here will effect the channel order for
 * audio_pipeline_input() and audio_pipeline_output()
 */
typedef struct {
    int32_t samples[appconfAUDIO_PIPELINE_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    int32_t mic_samples_passthrough[appconfAUDIO_PIPELINE_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    uint8_t vad;
} frame_data_t;

#if appconfAUDIO_PIPELINE_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

typedef struct ic_stage_ctx {
    ic_state_t state;
} ic_stage_ctx_t;

typedef struct vad_stage_ctx {
    vad_state_t state;
} vad_stage_ctx_t;

typedef struct ns_stage_ctx {
    ns_state_t state;
} ns_stage_ctx_t;

typedef struct agc_stage_ctx {
    agc_meta_data_t md;
    agc_state_t state;
} agc_stage_ctx_t;

static ic_stage_ctx_t ic_stage_state = {};
static vad_stage_ctx_t vad_stage_state = {};
static ns_stage_ctx_t ns_stage_state = {};
static agc_stage_ctx_t agc_stage_state = {};

static void *audio_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));

    audio_pipeline_input(input_app_data,
                       (int32_t **)frame_data->samples,
                       2,
                       appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    frame_data->vad = 0;

    memcpy(frame_data->mic_samples_passthrough, frame_data->samples, sizeof(frame_data->mic_samples_passthrough));

    return frame_data;
}

static int audio_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    return audio_pipeline_output(output_app_data,
                               (int32_t **)frame_data->samples,
                               4,
                               appconfAUDIO_PIPELINE_FRAME_ADVANCE);
}

static void stage_vad_and_ic(frame_data_t *frame_data)
{
    int32_t ic_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    ic_filter(&ic_stage_state.state,
              frame_data->samples[0],
              frame_data->samples[1],
              ic_output);
    uint8_t vad = vad_probability_voice(ic_output, &vad_stage_state.state);
    ic_adapt(&ic_stage_state.state, vad, ic_output);
    frame_data->vad = vad;
    memcpy(frame_data->samples, ic_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
}

static void stage_ns(frame_data_t *frame_data)
{
    int32_t ns_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(NS_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    ns_process_frame(
                &ns_stage_state.state,
                ns_output,
                frame_data->samples[0]);
    memcpy(frame_data->samples, ns_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
}

static void stage_agc(frame_data_t *frame_data)
{
    int32_t agc_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(AGC_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    agc_stage_state.md.vad_flag = (frame_data->vad > AGC_VAD_THRESHOLD);

    agc_process_frame(
            &agc_stage_state.state,
            agc_output,
            frame_data->samples[0],
            &agc_stage_state.md);
    memcpy(frame_data->samples, agc_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
}

static void initialize_pipeline_stages(void) {
    ic_init(&ic_stage_state.state);
    vad_init(&vad_stage_state.state);
    ns_init(&ns_stage_state.state);
    agc_init(&agc_stage_state.state, &AGC_PROFILE_ASR);
    agc_stage_state.md.vad_flag = AGC_META_DATA_NO_VAD;
    agc_stage_state.md.aec_ref_power = AGC_META_DATA_NO_AEC;
    agc_stage_state.md.aec_corr_factor = AGC_META_DATA_NO_AEC;
}

void audio_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
    const int stage_count = 3;

    const pipeline_stage_t stages[] = {
        (pipeline_stage_t)stage_vad_and_ic,
        (pipeline_stage_t)stage_ns,
        (pipeline_stage_t)stage_agc,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_vad_and_ic) + RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_ns),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_agc) + RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
    };

    initialize_pipeline_stages();

    generic_pipeline_init((pipeline_input_t)audio_pipeline_input_i,
                        (pipeline_output_t)audio_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        appconfAUDIO_PIPELINE_TASK_PRIORITY,
                        stage_count);
}
