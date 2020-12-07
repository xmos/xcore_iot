
// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model_runner.h"

#ifdef XCORE
#define ATTRIBUTE_EXTMEM_SECTION __attribute__((section(".ExtMem_data")))
#else  // not XCORE
#define ATTRIBUTE_EXTMEM_SECTION
#endif

// USE RAM
// #define MAX_MODEL_CONTENT_SIZE 50000
// unsigned char model_content[MAX_MODEL_CONTENT_SIZE];
// #define TENSOR_ARENA_SIZE 125000
// unsigned char tensor_arena[TENSOR_ARENA_SIZE];

// USE DDR
#define MAX_MODEL_CONTENT_SIZE 500000
ATTRIBUTE_EXTMEM_SECTION unsigned char model_content[MAX_MODEL_CONTENT_SIZE];
#define TENSOR_ARENA_SIZE 200000
unsigned char tensor_arena[TENSOR_ARENA_SIZE];

static size_t input_size;
static unsigned char *input_buffer;

static size_t output_size;
static unsigned char *output_buffer;

static int load_model(const char *filename, unsigned char *content,
                      size_t *size) {
  FILE *fd = fopen(filename, "rb");
  fseek(fd, 0, SEEK_END);
  size_t fsize = ftell(fd);

  fseek(fd, 0, SEEK_SET);
  fread(content, 1, fsize, fd);
  fclose(fd);

  *size = fsize;

  return 1;
}

static int load_input(const char *filename, unsigned char *input,
                      size_t esize) {
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

static int save_output(const char *filename, const unsigned char *output,
                       size_t osize) {
  FILE *fd = fopen(filename, "wb");
  fwrite(output, sizeof(int8_t), osize, fd);
  fclose(fd);

  return 1;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Three arguments expected: mode.tflite input-file output-file\n");
    return -1;
  }

  char *model_filename = argv[1];
  char *input_filename = argv[2];
  char *output_filename = argv[3];

  // load model
  size_t model_size;
  if (!load_model(model_filename, model_content, &model_size)) {
    printf("error loading model filename=%s\n", model_filename);
    return -1;
  }
  // setup runtime
  model_runner_init(model_content, tensor_arena, TENSOR_ARENA_SIZE,
                    &input_buffer, &input_size, &output_buffer, &output_size);

  // Load input tensor
  if (!load_input(input_filename, input_buffer, input_size)) {
    printf("error loading input filename=%s\n", input_filename);
    return -1;
  }

  // Run inference, and report any error
  model_runner_invoke();

  // save output
  if (!save_output(output_filename, output_buffer, output_size)) {
    printf("error saving output filename=%s\n", output_filename);
    return -1;
  }
  return 0;
}
