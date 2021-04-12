// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _TIME_H_
#define _clock_defined
#endif
#include <xcore/thread.h>

#include "test_model_data.h"
#include "test_model_runner.h"
#include "xcore_device_memory.h"

#if defined(USE_SWMEM)
__attribute__((aligned(8))) static char swmem_handler_stack[1024];
#endif

#define TENSOR_ARENA_SIZE 100000  // this is big enough for all test models
static unsigned char tensor_arena[TENSOR_ARENA_SIZE];

static model_runner_t model_runner_ctx_s;
static model_runner_t *model_runner_ctx = &model_runner_ctx_s;

static int8_t *input_buffer;
static size_t input_size;
static int8_t *output_buffer;

int main(void) {
#if defined(USE_SWMEM)
  // start SW_Mem handler
  swmem_setup();
  size_t stack_words;
  GET_STACKWORDS(stack_words, swmem_handler);
  run_async(swmem_handler, NULL,
            stack_base(swmem_handler_stack, stack_words + 2));
#endif

  // initialize model runner global state
  model_runner_init(tensor_arena, TENSOR_ARENA_SIZE);

  // setup model runner
  test_model_runner_create(model_runner_ctx, NULL);
  model_runner_allocate(model_runner_ctx, test_model_data);
  input_buffer = model_runner_input_buffer_get(model_runner_ctx);
  input_size = model_runner_input_size_get(model_runner_ctx);
  output_buffer = model_runner_output_buffer_get(model_runner_ctx);

  // set input tensor to all zeros
  memset(input_buffer, 0, input_size);

  // Run inference, and report any error
  printf("Running inference...\n");
  model_runner_invoke(model_runner_ctx);

  model_runner_profiler_summary_print(model_runner_ctx);

  return 0;
}
