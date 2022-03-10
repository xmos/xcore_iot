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

typedef struct {
    ap_ch_pair_t samples[AP_CHANNEL_PAIRS][AP_FRAME_ADVANCE];
    int32_t samples_internal_fmt[AP_CHANNELS][AP_FRAME_ADVANCE];
    ap_ch_pair_t mic_samples_passthrough[AP_CHANNELS][AP_FRAME_ADVANCE];
} frame_data_t;

#if AP_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

#define AP_MAX_Y_CHANNELS (2)
#define AP_MAX_X_CHANNELS (2)

typedef struct ic_stage_ctx {
    ic_state_t state;
} ic_stage_ctx_t;

typedef struct vad_stage_ctx {
    vad_state_t state;
} vad_stage_ctx_t;

typedef struct ns_stage_ctx {
    ns_state_t state[AP_MAX_Y_CHANNELS];
} ns_stage_ctx_t;

typedef struct agc_stage_ctx {
    agc_meta_data_t md;
    agc_state_t state[AP_CHANNELS];
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
                       (int32_t(*)[2])frame_data->samples,
                       AP_FRAME_ADVANCE);

    memcpy(frame_data->mic_samples_passthrough, frame_data->samples, sizeof(frame_data->mic_samples_passthrough));

    /* Convert to audiopipeline specific format */
    for(int i=0; i<AP_FRAME_ADVANCE; i++) {
        frame_data->samples_internal_fmt[0][i] = frame_data->samples[0][i].ch_a;
        frame_data->samples_internal_fmt[1][i] = frame_data->samples[0][i].ch_b;
    }

    return frame_data;
}

static int audio_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    /* Convert out of audiopipeline specific format */
    for(int i=0; i<AP_FRAME_ADVANCE; i++) {
        frame_data->samples[0][i].ch_a = frame_data->samples_internal_fmt[0][i];
        frame_data->samples[0][i].ch_b = frame_data->samples_internal_fmt[1][i];
    }

    return audio_pipeline_output(output_app_data,
                               (int32_t(*)[2])frame_data->samples,
                               (int32_t(*)[2])frame_data->mic_samples_passthrough,
                               AP_FRAME_ADVANCE);
}

static void stage_vad_and_ic(frame_data_t *frame_data)
{
    int32_t ic_output[AP_CHANNELS][AP_FRAME_ADVANCE];
    ic_filter(&ic_stage_state.state,
        frame_data->samples_internal_fmt[0],
        frame_data->samples_internal_fmt[1],
        ic_output[0]);
    uint8_t vad = vad_probability_voice(ic_output[0], &vad_stage_state.state);
    ic_adapt(&ic_stage_state.state, vad, ic_output[0]);
    memcpy(frame_data->samples_internal_fmt[0], ic_output[0], AP_FRAME_ADVANCE * sizeof(int32_t));   /* Check if it is safe to do ic_output inplace */
}

static void stage_ns(frame_data_t *frame_data)
{
    configASSERT(NS_FRAME_ADVANCE == AP_FRAME_ADVANCE);

    for(int ch=0; ch<AP_CHANNELS; ch++) {
        ns_process_frame(
                    &ns_stage_state.state[ch],
                    frame_data->samples_internal_fmt[ch],
                    frame_data->samples_internal_fmt[ch]);
    }
}

static void stage_agc(frame_data_t *frame_data)
{
    configASSERT(AGC_FRAME_ADVANCE == AP_FRAME_ADVANCE);

    for(int ch=0; ch<AP_CHANNELS; ch++) {
        agc_process_frame(
                &agc_stage_state.state[ch],
                frame_data->samples_internal_fmt[ch],
                frame_data->samples_internal_fmt[ch],
                &agc_stage_state.md);
    }
}

static void initialize_pipeline_stages(void) {
    ic_init(&ic_stage_state.state);
    vad_init(&vad_stage_state.state);
    for(int ch = 0; ch < AP_MAX_Y_CHANNELS; ch++){
        ns_init(&ns_stage_state.state[ch]);
        agc_init(&agc_stage_state.state[ch], &AGC_PROFILE_ASR);
    }
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
