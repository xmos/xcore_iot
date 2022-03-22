// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef STAGE1_STATE_H
#define STAGE1_STATE_H

#include "aec_api.h"
#include "audio_pipeline/aec_memory_pool.h"
#include "adec_api.h"
#include "audio_pipeline/delay_buffer/delay_buffer.h"


typedef struct {
    uint8_t num_x_channels;
    uint8_t num_y_channels;
    uint8_t num_main_filt_phases;
    uint8_t num_shadow_filt_phases;
} aec_conf_t;

typedef struct {
    // AEC
    aec_state_t DWORD_ALIGNED aec_main_state;
    aec_state_t DWORD_ALIGNED aec_shadow_state;
    aec_shared_state_t DWORD_ALIGNED aec_shared_state;
    uint8_t DWORD_ALIGNED aec_main_memory_pool[sizeof(aec_memory_pool_t)];
    uint8_t DWORD_ALIGNED aec_shadow_memory_pool[sizeof(aec_shadow_filt_memory_pool_t)];

    // ADEC
    adec_state_t DWORD_ALIGNED adec_state;

    // Delay Buffer
    delay_buf_state_t DWORD_ALIGNED delay_state;

    //Top level
    aec_conf_t aec_de_mode_conf;
    aec_conf_t aec_non_de_mode_conf;
    int32_t delay_estimator_enabled;
    float_s32_t ref_active_threshold; //-60dB
} stage_1_state_t;

void stage_1_init(stage_1_state_t *state, aec_conf_t *de_conf, aec_conf_t *non_de_conf, adec_config_t *adec_config);
void stage_1_process_frame(stage_1_state_t *state, int32_t (*output_frame)[AP_FRAME_ADVANCE], float_s32_t *max_ref_energy, float_s32_t *aec_corr_factor, int32_t (*input_y)[AP_FRAME_ADVANCE], int32_t (*input_x)[AP_FRAME_ADVANCE]);
#endif
