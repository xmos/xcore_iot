// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.
#include <stdio.h>
#include <string.h>

#include "inference_engine.h"

static int input_bytes = 0;
static int input_size;
static unsigned char *input_buffer;
static int output_size;
static unsigned char *output_buffer;

void print_output() {
  for (int i = 0; i < output_size; i++) {
    printf("Output index=%u, value=%i\n", i, (signed char)output_buffer[i]);
  }
  printf("DONE!\n");
}

void app_main() {
  initialize(&input_buffer, &input_size, &output_buffer, &output_size);
}

void app_data(void *data, size_t size) {
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size) {
    invoke();
    print_profiler_summary();
    print_output();
    input_bytes = 0;
  }
}
