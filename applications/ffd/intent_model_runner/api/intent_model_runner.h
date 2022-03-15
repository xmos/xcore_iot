// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef INTENT_MODEL_RUNNER_H_
#define INTENT_MODEL_RUNNER_H_

/* Use this api if model runner and sample sending are on the same tile */
void intent_task_create(unsigned priority);
void intent_samples_send(size_t frame_count,
                         int32_t (*processed_audio_frame)[2]);

/* Use this api if model runner and sample sending are not on the same tile */
void intertile_intent_task_create(unsigned priority);
void intent_intertile_samples_send(rtos_intertile_t *intertile_ctx,
                                   size_t frame_count,
                                   int32_t (*processed_audio_frame)[2]);

void model_runner_manager(void *args);

#endif /* INTENT_MODEL_RUNNER_H_ */
