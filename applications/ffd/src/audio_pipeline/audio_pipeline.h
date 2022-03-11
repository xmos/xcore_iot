// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

#include <stdint.h>
#include "app_conf.h"

typedef struct {
    int32_t ch_a;
    int32_t ch_b;
} ap_ch_pair_t;

#define AP_FRAME_ADVANCE          240
#define AP_CHANNEL_PAIRS          1
#define AP_CHANNELS               2

#define AUDIO_PIPELINE_AUDIO_SAMPLE_RATE  appconfAUDIO_PIPELINE_SAMPLE_RATE
#define AUDIO_PIPELINE_AUDIO_FRAME_LENGTH appconfAUDIO_PIPELINE_FRAME_ADVANCE

#define AUDIO_PIPELINE_DONT_FREE_FRAME 0
#define AUDIO_PIPELINE_FREE_FRAME      1

void audio_pipeline_init(
        void *input_app_data,
        void *output_app_data);

void audio_pipeline_input(
        void *input_app_data,
        int32_t (*mic_audio_frame)[2],
        size_t frame_count);

int audio_pipeline_output(
        void *output_app_data,
        int32_t (*proc_audio_frame)[2],
        int32_t (*debug_audio_frame)[2],
        int32_t (*mic_audio_frame)[2],
        size_t frame_count);

#endif /* AUDIO_PIPELINE_H_ */
