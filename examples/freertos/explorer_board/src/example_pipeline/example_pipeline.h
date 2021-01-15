// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SRC_EXAMPLE_PIPELINE_H_
#define SRC_EXAMPLE_PIPELINE_H_

#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"
#include "drivers/rtos/i2s/FreeRTOS/rtos_i2s_master.h"

#define EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE 48000
#define EXAMPLE_PIPELINE_AUDIO_FRAME_LENGTH MIC_DUAL_FRAME_SIZE

void example_pipeline_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx);

#endif /* SRC_EXAMPLE_PIPELINE_H_ */
