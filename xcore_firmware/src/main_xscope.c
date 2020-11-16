
// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inference_engine.h"
#include "xscope.h"

#define LOG_ERROR(fmt, args...) printf(fmt, ##args)
#define LOG_STATUS(fmt, args...)  // printf(fmt, ##args)

// USE DDR
#define MAX_MODEL_CONTENT_SIZE 500000
__attribute__((section(
    ".ExtMem_data"))) unsigned char model_content[MAX_MODEL_CONTENT_SIZE];
unsigned char model_content[MAX_MODEL_CONTENT_SIZE];
static size_t model_received_bytes = 0;
#define TENSOR_ARENA_SIZE 215000
unsigned char tensor_arena[TENSOR_ARENA_SIZE];

static size_t model_size = 0;

static int tensor_index = -1;
static size_t tensor_size = 0;
static size_t tensor_received_bytes = 0;

static size_t input_size;
static unsigned char *input_buffer;

static size_t output_size;
static unsigned char *output_buffer;

enum AppState { Model, Initialize, SetTensor, Invoke, GetTensor };
static enum AppState state;

void xscope_main() {}

void send_error(char *message) {
  xscope_bytes(ERROR, strlen(message), (const unsigned char *)message);
}

void send_tensor(void *buffer, size_t size) {
  xscope_bytes(GET_TENSOR, size, (const unsigned char *)buffer);
}

void xscope_data(void *data, size_t size) {
  void *tensor_buffer;

  // Handle state protocol messages
  if (strncmp(data, "PING_RECV", 9) == 0) {
    LOG_STATUS("Received PING_RECV\n");
    xscope_int(PING_ACK, 0);
    return;
  } else if (strncmp(data, "SET_MODEL", 9) == 0) {
    LOG_STATUS("Received SET_MODEL\n");
    state = Model;
    reset_inference_engine(tensor_arena, TENSOR_ARENA_SIZE);
    model_received_bytes = 0;
    sscanf(data, "SET_MODEL %d", &model_size);
    if (model_size > MAX_MODEL_CONTENT_SIZE) {
      LOG_ERROR("Model exceeds maximum size of %d bytes\n",
                MAX_MODEL_CONTENT_SIZE);
      send_error("Model exceeds maximum size\0");
    }
    return;
  } else if (strncmp(data, "CALL_INITIALIZE", 15) == 0) {
    LOG_STATUS("Received CALL_INITIALIZE\n");
    // Note, initialize will log error if it fails
    state = Initialize;
    TfLiteStatus status = initialize_inference_engine(
        model_content, tensor_arena, TENSOR_ARENA_SIZE, &input_buffer,
        &input_size, &output_buffer, &output_size);
    if (status == kTfLiteError) {
      send_error(
          "Unable to initialize inference engine. Check tensor arena size.\0");
    }
    xscope_int(INIT_ACK, 0);
    return;
  } else if (strncmp(data, "SET_TENSOR", 9) == 0) {
    LOG_STATUS("Received SET_TENSOR\n");
    state = SetTensor;
    tensor_received_bytes = 0;
    sscanf(data, "SET_TENSOR %d %d", &tensor_index, &tensor_size);
    LOG_STATUS("SET_TENSOR index=%d   size=%d\n", tensor_index, tensor_size);
    return;
  } else if (strncmp(data, "GET_TENSOR", 9) == 0) {
    LOG_STATUS("Received GET_TENSOR\n");
    state = GetTensor;
    sscanf(data, "GET_TENSOR %d", &tensor_index);
    get_tensor_bytes(tensor_index, &tensor_buffer, &tensor_size);
    LOG_STATUS("GET_TENSOR index=%d  size=%d\n", tensor_index, tensor_size);
    send_tensor(tensor_buffer, tensor_size);
    return;
  } else if (strncmp(data, "CALL_INVOKE", 11) == 0) {
    LOG_STATUS("Received INVOKE\n");
    state = Invoke;
    TfLiteStatus status = invoke_inference_engine();
    if (status == kTfLiteError) {
      send_error("Unable to invoke inference engine.\0");
    }
    xscope_int(INVOKE_ACK, 0);
    return;
  }

  // Handle data payload messages
  switch (state) {
    case Model:
      memcpy(model_content + model_received_bytes, data, size);
      model_received_bytes += size;
      if (model_received_bytes > model_size) {
        send_error("Too many bytes received for model\0");
      }
      xscope_int(RECV_ACK, 0);
      break;
    case SetTensor:
      get_tensor_bytes(tensor_index, &tensor_buffer, &tensor_size);
      memcpy(tensor_buffer + tensor_received_bytes, data, size);
      tensor_received_bytes += size;
      if (tensor_received_bytes > tensor_size) {
        LOG_ERROR("Tensor exceeds size of %d bytes\n", tensor_size);
        send_error("Too many bytes received for tensor\0");
      }
      xscope_int(RECV_ACK, 0);
      break;
    case Initialize:
    case Invoke:
    case GetTensor:
      break;
  }
}
