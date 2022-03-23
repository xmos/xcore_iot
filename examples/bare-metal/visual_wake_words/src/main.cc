// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <string.h>

#include "vww_model_data.h"
#include "xtflm_conf.h"
#include "inference_engine.h"

#define TENSOR_ARENA_SIZE_BYTES 100000
static uint64_t tensor_arena[TENSOR_ARENA_SIZE_BYTES/sizeof(uint64_t)];

static inference_engine_t ie;
static struct tflite_micro_objects tflmo;

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

#ifdef __cplusplus
extern "C" {
#endif

void app_main()
{
  // setup inference engine
  auto *resolver = inference_engine_initialize(&ie, (uint32_t *)tensor_arena, TENSOR_ARENA_SIZE_BYTES, (uint32_t *) vww_model_data, vww_model_data_len, &tflmo);
  resolver->AddSoftmax();
  resolver->AddPad();
  resolver->AddConv2D();
  resolver->AddReshape();
  resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_V2_OpCode,
                      tflite::ops::micro::xcore::Register_Conv2D_V2());
  inference_engine_load_model(&ie, vww_model_data_len, (uint32_t *) vww_model_data, 0);

  input_buffer = (int8_t *) ie.input_buffers[0];
  input_size = ie.input_sizes[0];
  output_buffer = (int8_t *) ie.output_buffers[0];
  output_size = ie.output_sizes[0];
}

void app_data(void *data, size_t size)
{
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size)
  {
    interp_invoke_par_5(&ie);
    print_profiler_summary(&ie);

    print_output();
    input_bytes = 0;
  }
}

#ifdef __cplusplus
};
#endif