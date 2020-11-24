
// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cifar10_model_data.h"
#include "cifar10_model_runner.h"
#include "xcore_device_memory.h"

#if defined(USE_SWMEM)
__attribute__((aligned(8))) static char swmem_handler_stack[1024];
#endif

#define TENSOR_ARENA_SIZE \
  58000  // NOTE: This is big enough fo the model to live in RAM or external
         // memory
static unsigned char tensor_arena[TENSOR_ARENA_SIZE];

static model_runner_t model_runner_ctx_s;
static model_runner_t *model_runner_ctx = &model_runner_ctx_s;
static int8_t *input_buffer;
static size_t input_size;
static int8_t *output_buffer;

static int argmax(const int8_t *A, const int N) {
  int m = 0;

  for (int i = 1; i < N; i++) {
    if (A[i] > A[m]) {
      m = i;
    }
  }

  return m;
}

static int load_test_input(const char *filename, int8_t *input, size_t esize) {
  FILE *fd = fopen(filename, "rb");
  fseek(fd, 0, SEEK_END);
  size_t fsize = ftell(fd);

  if (fsize != esize) {
    printf("Incorrect input file size. Expected %d bytes.\n", esize);
    return 0;
  }

  fseek(fd, 0, SEEK_SET);
  fread(input, 1, fsize, fd);
  fclose(fd);

  return 1;
}

int main(int argc, char *argv[]) {
#if defined(USE_SWMEM)
  // start SW_Mem handler
  swmem_setup();
  size_t stack_words;
  GET_STACKWORDS(stack_words, swmem_handler);
  run_async(swmem_handler, NULL,
            stack_base(swmem_handler_stack, stack_words + 2));
#endif

  // setup model runner
  model_runner_init(model_runner_ctx, cifar10_model_data, tensor_arena,
                    TENSOR_ARENA_SIZE);
  input_buffer = model_runner_get_input(model_runner_ctx);
  input_size = model_runner_get_input_size(model_runner_ctx);
  output_buffer = model_runner_get_output(model_runner_ctx);

  if (argc > 1) {
    printf("Input filename = %s\n", argv[1]);
    // Load input tensor
    if (!load_test_input(argv[1], input_buffer, input_size)) return -1;
  } else {
    printf("No input file\n");
    memset(input_buffer, 0, input_size);
  }

  // Run inference, and report any error
  printf("Running inference...\n");
  model_runner_invoke(model_runner_ctx);

  char classification[12] = {0};
  int m = argmax(output_buffer, 10);

  switch (m) {
    case 0:
      snprintf(classification, 9, "Airplane");
      break;
    case 1:
      snprintf(classification, 11, "Automobile");
      break;
    case 2:
      snprintf(classification, 5, "Bird");
      break;
    case 3:
      snprintf(classification, 4, "Cat");
      break;
    case 4:
      snprintf(classification, 5, "Deer");
      break;
    case 5:
      snprintf(classification, 4, "Dog");
      break;
    case 6:
      snprintf(classification, 5, "Frog");
      break;
    case 7:
      snprintf(classification, 6, "Horse");
      break;
    case 8:
      snprintf(classification, 5, "Ship");
      break;
    case 9:
      snprintf(classification, 6, "Truck");
      break;
    default:
      break;
  }
  printf("Classification = %s\n", classification);

  return 0;
}
