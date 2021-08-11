// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <string.h>

#include "vww_model_data.h"
#include "vww_model_runner.h"

#define TENSOR_ARENA_SIZE 100000
static unsigned char tensor_arena[TENSOR_ARENA_SIZE];

static model_runner_t model_runner_ctx_s;
static model_runner_t *model_runner_ctx = &model_runner_ctx_s;

static int8_t *input_buffer;
static size_t input_size;
static int8_t *output_buffer;
static size_t output_size;
static int input_bytes = 0;

void print_output()
{
  for (int i = 0; i < output_size; i++)
  {
    printf("Output index=%u, value=%i\n", i, (signed char)output_buffer[i]);
  }
  printf("DONE!\n");
}

void app_main()
{
  // initialize model runner global state
  model_runner_init(tensor_arena, TENSOR_ARENA_SIZE);

  // setup model runner
  vww_model_runner_create(model_runner_ctx, NULL);
  model_runner_allocate(model_runner_ctx, vww_model_data);
  input_buffer = model_runner_input_buffer_get(model_runner_ctx);
  input_size = model_runner_input_size_get(model_runner_ctx);
  output_buffer = model_runner_output_buffer_get(model_runner_ctx);
  output_size = model_runner_output_size_get(model_runner_ctx);
}

void app_data(void *data, size_t size)
{
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size)
  {
    model_runner_invoke(model_runner_ctx);

    model_runner_profiler_summary_print(model_runner_ctx);
    print_output();
    input_bytes = 0;
  }
}
