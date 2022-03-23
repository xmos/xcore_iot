// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <string.h>

#include "vww_model_data.h"
#include "person_classifier.h"

#define TENSOR_ARENA_SIZE 100000
static unsigned char tensor_arena[TENSOR_ARENA_SIZE];

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
  person_classifier_init(tensor_arena, TENSOR_ARENA_SIZE);

  // setup model runner
  person_classifier_allocate(vww_model_data);
  input_buffer = person_classifier_input_buffer_get();
  input_size = person_classifier_input_size_get();
  output_buffer = person_classifier_output_buffer_get();
  output_size = person_classifier_output_size_get();
}

void app_data(void *data, size_t size)
{
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size)
  {
    person_classifier_invoke();

    person_classifier_profiler_summary_print();
    print_output();
    input_bytes = 0;
  }
}
