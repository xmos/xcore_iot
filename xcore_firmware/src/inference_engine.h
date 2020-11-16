// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "tensorflow/lite/c/common.h"

void reset_inference_engine(uint8_t *tensor_arena, size_t tensor_arena_size);
TfLiteStatus initialize_inference_engine(uint8_t *model_content,
                                         uint8_t *tensor_arena,
                                         size_t tensor_arena_size,
                                         uint8_t **input, size_t *input_size,
                                         uint8_t **output, size_t *output_size);
void get_tensor_bytes(int index, void **bytes, size_t *size);
TfLiteStatus invoke_inference_engine();

#ifdef __cplusplus
};
#endif

#endif  // INFERENCE_ENGINE_H_