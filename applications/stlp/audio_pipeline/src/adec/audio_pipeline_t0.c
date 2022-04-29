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
#include "stream_buffer.h"

/* Library headers */
#include "generic_pipeline.h"
#include "aec_api.h"
#include "agc_api.h"
#include "ic_api.h"
#include "ns_api.h"
#include "vad_api.h"
#include "adec_api.h"

/* App headers */
#include "app_conf.h"
#include "app_control/app_control.h"
#include "audio_pipeline.h"
#include "audio_pipeline_dsp.h"

#if appconfAUDIO_PIPELINE_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

#if ON_TILE(0)
static ic_stage_ctx_t ic_stage_state = {};
static vad_stage_ctx_t vad_stage_state = {};
static ns_stage_ctx_t ns_stage_state = {};
static agc_stage_ctx_t agc_stage_state = {};

static void *audio_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));
    memset(frame_data, 0x00, sizeof(frame_data_t));

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

    return frame_data;
}

static int audio_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    return audio_pipeline_output(output_app_data,
                               (int32_t **)frame_data->samples,
                               6,
                               appconfAUDIO_PIPELINE_FRAME_ADVANCE);
}

static void stage_vad_and_ic(frame_data_t *frame_data)
{
#if appconfAUDIO_PIPELINE_SKIP_IC_AND_VAD
#else
    int32_t ic_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    ic_filter(&ic_stage_state.state,
              frame_data->samples[0],
              frame_data->samples[1],
              ic_output);
    uint8_t vad = vad_probability_voice(ic_output, &vad_stage_state.state);
    ic_adapt(&ic_stage_state.state, vad, ic_output);
    frame_data->vad = vad;
    /* Intentionally ignoring comms ch from here on out */
    memcpy(frame_data->samples, ic_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
#endif
}

static void stage_ns(frame_data_t *frame_data)
{
#if appconfAUDIO_PIPELINE_SKIP_NS
#else
    int32_t ns_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(NS_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    ns_process_frame(
                &ns_stage_state.state,
                ns_output,
                frame_data->samples[0]);
    memcpy(frame_data->samples, ns_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
#endif
}

static void stage_agc(frame_data_t *frame_data)
{
#if appconfAUDIO_PIPELINE_SKIP_AGC
#else
    int32_t agc_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(AGC_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    agc_stage_state.md.vad_flag = (frame_data->vad > AGC_VAD_THRESHOLD);
    agc_stage_state.md.aec_ref_power = frame_data->max_ref_energy;
    agc_stage_state.md.aec_corr_factor = frame_data->aec_corr_factor;

    agc_process_frame(
            &agc_stage_state.state,
            agc_output,
            frame_data->samples[0],
            &agc_stage_state.md);
    memcpy(frame_data->samples, agc_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
#endif
}

static void initialize_pipeline_stages(void)
{
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
}

#endif /* ON_TILE(0)*/
