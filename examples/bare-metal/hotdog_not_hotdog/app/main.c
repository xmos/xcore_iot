// Copyright 2019 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1
#include <stdio.h>
#include <string.h>

#include "inference_engine.h"

// Include xCORE port functions
#include <xcore/port.h>
#define LED_PORT XS1_PORT_4C

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

void show_result() {
  if (output_buffer[0] > output_buffer[1]) {
    // It's a hotdog! illuminate all LEDs
    port_out(LED_PORT, 0xffff);
  } else {
    // not a hotdog :(. Turn off LEDs
    port_out(LED_PORT, 0);
  }
}

void app_main() {
  printf("Initialising app\n");
  initialize(&input_buffer, &input_size, &output_buffer, &output_size);
  port_enable(LED_PORT);
  port_out(LED_PORT, 0);
}

void app_data(void *data, size_t size) {
  memcpy(input_buffer + input_bytes, data, size - 1);
  input_bytes += size - 1;
  if (input_bytes == input_size) {
    invoke();
    print_profiler_summary();
    print_output();
    show_result();
    input_bytes = 0;
  }
}
