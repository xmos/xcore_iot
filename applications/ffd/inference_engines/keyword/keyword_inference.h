// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef KEYWORD_INFERENCE_H_
#define KEYWORD_INFERENCE_H_

#include "stream_buffer.h"
#include "event_groups.h"

#define INFERENCE_BIT_SPOTTED_SILENCE (1<<0)
#define INFERENCE_BIT_SPOTTED_UNKNOWN (1<<1)
#define INFERENCE_BIT_CLEAR           (1<<2)
#define INFERENCE_BIT_COMPUTER        (1<<3)
#define INFERENCE_BIT_DISPLAY         (1<<4)
#define INFERENCE_BIT_GREEN           (1<<5)
#define INFERENCE_BIT_RED             (1<<6)

#define INFERENCE_BIT_ALL (  INFERENCE_BIT_SPOTTED_SILENCE     \
                           | INFERENCE_BIT_SPOTTED_UNKNOWN     \
                           | INFERENCE_BIT_CLEAR               \
                           | INFERENCE_BIT_COMPUTER            \
                           | INFERENCE_BIT_DISPLAY             \
                           | INFERENCE_BIT_GREEN               \
                           | INFERENCE_BIT_RED)

typedef struct keyword_engine_args {
    StreamBufferHandle_t samples_to_engine_stream_buf;
    EventGroupHandle_t egrp_inference;
} keyword_engine_args_t;

#ifdef __cplusplus
extern "C" {
#endif

void keyword_engine_task_create(
        uint32_t priority,
        keyword_engine_args_t *args);
void keyword_engine_samples_send_local(
        size_t frame_count,
        int32_t *processed_audio_frame);

void keyword_engine_intertile_task_create(
        uint32_t priority,
        keyword_engine_args_t *args);
void keyword_engine_samples_send_remote(
        rtos_intertile_t *intertile_ctx,
        size_t frame_count,
        int32_t *processed_audio_frame);

#ifdef __cplusplus
};
#endif

#endif /* KEYWORD_INFERENCE_H_ */
