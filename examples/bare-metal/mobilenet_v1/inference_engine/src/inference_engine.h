// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

void initialize(unsigned char **input, int *input_size, unsigned char **output,
                int *output_size);
void invoke();
void print_profiler_summary();

#ifdef __cplusplus
};
#endif

#endif  // INFERENCE_ENGINE_H_
