// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef TEMPLATE_INF_ENG_H_
#define TEMPLATE_INF_ENG_H_

void template_engine_task(void *args);

void template_engine_task_create(unsigned priority);
void template_engine_samples_send_local(
        size_t frame_count,
        int32_t *processed_audio_frame);

void template_engine_intertile_task_create(uint32_t priority);
void template_engine_samples_send_remote(
        rtos_intertile_t *intertile_ctx,
        size_t frame_count,
        int32_t *processed_audio_frame);

#endif /* TEMPLATE_INF_ENG_H_ */
