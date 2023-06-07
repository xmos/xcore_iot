// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcore/chanend.h>

#include "vww_model.h"

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

extern "C" {

#pragma stackfunction 1000  
void app_data(unsigned char *data, size_t size)
{
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size)
  {
    TfLiteStatus status = model_invoke();
    if (status != kTfLiteOk) {
      printf("ERROR: model_invoke returned %d\n", status);
      _Exit(1);
    }
    
    print_output();
    input_bytes = 0;
  }
}

#pragma stackfunction 1000  
void app_init(chanend_t flash_server)
{
  // Initialize the model
  TfLiteStatus status = model_init((void *)flash_server);
  if (status != kTfLiteOk) {
    printf("ERROR: model_init returned %d\n", status);
    _Exit(1);
  }

  // Setup model input and output buffers
  input_buffer = model_input(0)->data.int8;
  input_size = model_input_size(0);
  output_buffer = model_output(0)->data.int8;
  output_size = model_output_size(0);

  printf("model initialized\n");
}


}