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
#include "aec_api.h"
#include "agc_api.h"
#include "ic_api.h"
#include "ns_api.h"
#include "vad_api.h"
#include "audio_pipeline/aec_memory_pool.h"
#include "adec_api.h"
#include "audio_pipeline/delay_buffer/delay_buffer.h"
#include "audio_pipeline/pipeline_stage_1/stage_1.h"

/* App headers */
#include "app_conf.h"
#include "app_control/app_control.h"
#include "audio_pipeline/audio_pipeline.h"

/* Note: Changing the order here will effect the channel order for
 * audio_pipeline_input() and audio_pipeline_output()
 */
typedef struct {
    int32_t samples[appconfAUDIO_PIPELINE_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    int32_t aec_reference_audio_samples[appconfAUDIO_PIPELINE_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    int32_t mic_samples_passthrough[appconfAUDIO_PIPELINE_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    uint8_t vad;
    float_s32_t max_ref_energy;
    float_s32_t aec_corr_factor;
} frame_data_t;

#if appconfAUDIO_PIPELINE_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

typedef struct stage1_ctx {
    stage_1_state_t DWORD_ALIGNED state;

    aec_conf_t aec_de_mode_conf;
    aec_conf_t aec_non_de_mode_conf;
    adec_config_t adec_conf;
} stage1_ctx_t;

typedef struct ic_stage_ctx {
    ic_state_t DWORD_ALIGNED state;
} ic_stage_ctx_t;

typedef struct vad_stage_ctx {
    vad_state_t DWORD_ALIGNED state;
} vad_stage_ctx_t;

typedef struct ns_stage_ctx {
    ns_state_t DWORD_ALIGNED state;
} ns_stage_ctx_t;

typedef struct agc_stage_ctx {
    agc_meta_data_t md;
    agc_state_t DWORD_ALIGNED state;
} agc_stage_ctx_t;

#if ON_TILE(1)
static stage1_ctx_t stage1_state = {};
#endif

static ic_stage_ctx_t ic_stage_state = {};
static vad_stage_ctx_t vad_stage_state = {};
static ns_stage_ctx_t ns_stage_state = {};
static agc_stage_ctx_t agc_stage_state = {};

static void *audio_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));

#if ON_TILE(0)
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfAUDIOPIPELINE_PORT,
            portMAX_DELAY);

    xassert(bytes_received == sizeof(frame_data_t));

    rtos_intertile_rx_data(
            intertile_ctx,
            frame_data,
            bytes_received);
#endif

#if ON_TILE(1)
    audio_pipeline_input(input_app_data,
                       (int32_t **)frame_data->aec_reference_audio_samples,
                       4,
                       appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    frame_data->vad = 0;

    memcpy(frame_data->samples, frame_data->mic_samples_passthrough, sizeof(frame_data->samples));
#endif
    return frame_data;
}

static int audio_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{

#if ON_TILE(1)
    rtos_intertile_tx(intertile_ctx,
                      appconfAUDIOPIPELINE_PORT,
                      frame_data,
                      sizeof(frame_data_t));
    return AUDIO_PIPELINE_FREE_FRAME;
#endif
#if ON_TILE(0)
    return audio_pipeline_output(output_app_data,
                               (int32_t **)frame_data->samples,
                               6,
                               appconfAUDIO_PIPELINE_FRAME_ADVANCE);
#endif
}

static void stage_dummy(frame_data_t *frame_data)
{
}

#if ON_TILE(1)
static void stage_1(frame_data_t *frame_data)
{
    int32_t stage1_output[AP_MAX_Y_CHANNELS][appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    stage_1_process_frame(&stage1_state.state,
                          &stage1_output[0],
                          &frame_data->max_ref_energy,
                          &frame_data->aec_corr_factor,
                          frame_data->samples,
                          frame_data->aec_reference_audio_samples);
    memcpy(frame_data->samples, stage1_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
}
#endif

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
    agc_stage_state.md.aec_ref_power = frame_data->max_ref_energy;

    agc_process_frame(
            &agc_stage_state.state,
            agc_output,
            frame_data->samples[0],
            &agc_stage_state.md);
    memcpy(frame_data->samples, agc_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
}

static void initialize_pipeline_stages(void)
{
#if ON_TILE(1)
    stage1_state.aec_non_de_mode_conf.num_y_channels = AP_MAX_Y_CHANNELS;
    stage1_state.aec_non_de_mode_conf.num_x_channels = AP_MAX_X_CHANNELS;
    stage1_state.aec_non_de_mode_conf.num_main_filt_phases = AEC_MAIN_FILTER_PHASES;
    stage1_state.aec_non_de_mode_conf.num_shadow_filt_phases = AEC_SHADOW_FILTER_PHASES;

    stage1_state.aec_de_mode_conf.num_y_channels = 1;
    stage1_state.aec_de_mode_conf.num_x_channels = 1;
    stage1_state.aec_de_mode_conf.num_main_filt_phases = 30;
    stage1_state.aec_de_mode_conf.num_shadow_filt_phases = 0;

    stage1_state.adec_conf.bypass = 1; // Bypass automatic DE correction
    stage1_state.adec_conf.force_de_cycle_trigger = 1; // Force a delay correction cycle, so that delay correction happens once after initialisation. Make sure this is set back to 0 after adec has requested a transition into DE mode once, to stop any further delay correction (automatic or forced) by ADEC
    stage_1_init(&stage1_state.state, &stage1_state.aec_de_mode_conf, &stage1_state.aec_non_de_mode_conf, &stage1_state.adec_conf);
#endif
#if ON_TILE(0)
    ic_init(&ic_stage_state.state);
    vad_init(&vad_stage_state.state);
    ns_init(&ns_stage_state.state);
    agc_init(&agc_stage_state.state, &AGC_PROFILE_ASR);
    agc_stage_state.md.vad_flag = AGC_META_DATA_NO_VAD;
    agc_stage_state.md.aec_ref_power = AGC_META_DATA_NO_AEC;
    agc_stage_state.md.aec_corr_factor = AGC_META_DATA_NO_AEC;
#endif
}

void audio_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
#if ON_TILE(1)
    const int stage_count = 1;

    const pipeline_stage_t stages[] = {
        (pipeline_stage_t)stage_1,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        // configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_dummy) + RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i),
        // configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_1),
        // configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_dummy) + RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
            configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_1) + RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i) + RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
    };

    initialize_pipeline_stages();

    app_control_aec_servicer_register();

    generic_pipeline_init((pipeline_input_t)audio_pipeline_input_i,
                        (pipeline_output_t)audio_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        appconfAUDIO_PIPELINE_TASK_PRIORITY,
                        stage_count);
#endif
#if ON_TILE(0)
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

    app_control_stage1_servicer_register();
    app_control_stage2_servicer_register();

    generic_pipeline_init((pipeline_input_t)audio_pipeline_input_i,
                        (pipeline_output_t)audio_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        appconfAUDIO_PIPELINE_TASK_PRIORITY,
                        stage_count);
#endif
}
