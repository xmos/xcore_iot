// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

void initialize(const unsigned char *model_content, uint8_t *tensor_arena,
                size_t tensor_arena_size, uint8_t **input, size_t *input_size,
                uint8_t **output, size_t *output_size);
void invoke();

#ifdef __cplusplus
};
#endif

#endif  // INFERENCE_ENGINE_H_
