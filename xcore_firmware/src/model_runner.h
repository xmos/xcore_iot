// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "tensorflow/lite/c/common.h"

typedef enum ModelRunnerStatus {
  Ok = 0,
  ModelVersionError = 1,
  AllocateTensorsError = 2,
  InvokeError = 3
} ModelRunnerStatus;

void model_runner_reset(uint8_t *tensor_arena, size_t tensor_arena_size);

ModelRunnerStatus model_runner_init(uint8_t *model_content,
                                    uint8_t *tensor_arena,
                                    size_t tensor_arena_size, uint8_t **input,
                                    size_t *input_size, uint8_t **output,
                                    size_t *output_size);

void model_runner_tensor_bytes_get(int index, void **bytes, size_t *size);

ModelRunnerStatus model_runner_invoke();

void model_runner_profiler_times_get(uint32_t *count, const uint32_t **times);

#ifdef __cplusplus
};
#endif

#endif  // INFERENCE_ENGINE_H_