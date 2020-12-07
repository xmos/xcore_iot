// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "tensorflow/lite/c/common.h"

void model_runner_reset(uint8_t *tensor_arena, size_t tensor_arena_size);

TfLiteStatus model_runner_init(uint8_t *model_content, uint8_t *tensor_arena,
                               size_t tensor_arena_size, uint8_t **input,
                               size_t *input_size, uint8_t **output,
                               size_t *output_size);

void model_runner_get_tensor_bytes(int index, void **bytes, size_t *size);

TfLiteStatus model_runner_invoke();

void model_runner_get_profiler_times(uint32_t *count, const uint32_t **times);

#ifdef __cplusplus
};
#endif

#endif  // INFERENCE_ENGINE_H_