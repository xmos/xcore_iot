// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef EXAMPLE_PIPELINE_H_
#define EXAMPLE_PIPELINE_H_

#include <stdlib.h>
#include <stdint.h>
#include "app_conf.h"

#define EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE  appconfAUDIO_PIPELINE_SAMPLE_RATE
#define EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH appconfAUDIO_PIPELINE_FRAME_ADVANCE
#define EXAMPLE_PIPELINE_CHANNEL_PAIRS      appconfAUDIO_PIPELINE_CHANNEL_PAIRS
#define EXAMPLE_PIPELINE_FRAME_ADVANCE      appconfAUDIO_PIPELINE_FRAME_ADVANCE

#define EXAMPLE_PIPELINE_DONT_FREE_FRAME    0
#define EXAMPLE_PIPELINE_FREE_FRAME         1

void example_pipeline_init(
        void *input_app_data,
        void *output_app_data);

void example_pipeline_input(void *input_app_data,
                            int32_t (*mic_audio_frame)[2],
                            size_t frame_count);

int example_pipeline_output(void *output_app_data,
                            int32_t (*proc_audio_frame)[2],
                            int32_t (*mic_audio_frame)[2],
                            size_t frame_count);

#endif /* EXAMPLE_PIPELINE_H_ */
