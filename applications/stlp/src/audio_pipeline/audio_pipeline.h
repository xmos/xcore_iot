// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

#include <stdint.h>
#include "app_conf.h"

#define NUM_AEC_THREADS 1

#define AEC_MAX_Y_CHANNELS   (2)
#define AEC_MAX_X_CHANNELS   (2)
#define AEC_MAIN_FILTER_PHASES    (10)
#define AEC_SHADOW_FILTER_PHASES    (5)

#define AUDIO_PIPELINE_DONT_FREE_FRAME 0
#define AUDIO_PIPELINE_FREE_FRAME      1

#define AP_FRAME_ADVANCE appconfAUDIO_PIPELINE_FRAME_ADVANCE
#define AP_MAX_X_CHANNELS 2
#define AP_MAX_Y_CHANNELS 2

void audio_pipeline_init(
        void *input_app_data,
        void *output_app_data);

void audio_pipeline_input(
        void *input_app_data,
        int32_t **input_audio_frames,
        size_t ch_count,
        size_t frame_count);

int audio_pipeline_output(
        void *output_app_data,
        int32_t **output_audio_frames,
        size_t ch_count,
        size_t frame_count);

#endif /* AUDIO_PIPELINE_H_ */
