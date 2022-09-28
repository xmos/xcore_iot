// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DATA_PIPELINE_H_
#define DATA_PIPELINE_H_

#include <stdint.h>
#include "app_conf.h"

#define DATA_PIPELINE_DONT_FREE_FRAME 0
#define DATA_PIPELINE_FREE_FRAME      1

typedef struct {
    int32_t data[appconfFRAME_ADVANCE];
} frame_data_t;

void data_pipeline_init(
        void *input_app_data,
        void *output_app_data);

void data_pipeline_input(
        void *input_app_data,
        int8_t **input_data_frame,
        size_t frame_count);

int data_pipeline_output(
        void *output_app_data,
        int8_t **output_data_frame,
        size_t frame_count);

#endif /* DATA_PIPELINE_H_ */
