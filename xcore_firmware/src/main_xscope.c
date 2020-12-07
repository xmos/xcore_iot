
// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model_runner.h"
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

static void *tensor_buffer;

static const uint32_t *profiler_times;
static uint32_t profiler_times_count = 0;

enum AppState {
  Ping,
  SetModel,
  RecvModel,
  Initialize,
  SetTensor,
  RecvTensor,
  Invoke,
  GetTensor,
  GetProfilerTimes
};
static enum AppState state;

void xscope_main() {}

void send_error(char *message) {
  xscope_bytes(ERROR, strlen(message), (const unsigned char *)message);
}

void send_tensor(void *buffer, size_t size) {
  xscope_bytes(GET_TENSOR, size, (const unsigned char *)buffer);
}

void send_profiler_times(size_t count, const uint32_t *times) {
  xscope_bytes(GET_PROFILER_TIMES, (count * sizeof(uint32_t)),
               (const unsigned char *)times);
}

void query_protocol_state(void *data) {
  if (strncmp(data, "PING_RECV", 9) == 0) {
    LOG_STATUS("Received PING_RECV\n");
    state = Ping;
  } else if (strncmp(data, "SET_MODEL", 9) == 0) {
    LOG_STATUS("Received SET_MODEL\n");
    state = SetModel;
  } else if (strncmp(data, "CALL_INITIALIZE", 15) == 0) {
    LOG_STATUS("Received CALL_INITIALIZE\n");
    state = Initialize;
  } else if (strncmp(data, "SET_TENSOR", 9) == 0) {
    LOG_STATUS("Received SET_TENSOR\n");
    state = SetTensor;
  } else if (strncmp(data, "GET_TENSOR", 9) == 0) {
    LOG_STATUS("Received GET_TENSOR\n");
    state = GetTensor;
  } else if (strncmp(data, "CALL_INVOKE", 11) == 0) {
    LOG_STATUS("Received INVOKE\n");
    state = Invoke;
  } else if (strncmp(data, "GET_PROFILER_TIMES", 18) == 0) {
    LOG_STATUS("Received GET_PROFILER_TIMES\n");
    state = GetProfilerTimes;
  }
}

void xscope_data_recv(void *data, size_t size) {
  TfLiteStatus status;

  query_protocol_state(data);

  switch (state) {
    case Ping:
      xscope_int(PING_ACK, 0);
      break;
    case SetModel:
      model_runner_reset(tensor_arena, TENSOR_ARENA_SIZE);
      model_received_bytes = 0;
      sscanf(data, "SET_MODEL %d", &model_size);
      if (model_size > MAX_MODEL_CONTENT_SIZE) {
        LOG_ERROR("Model exceeds maximum size of %d bytes\n",
                  MAX_MODEL_CONTENT_SIZE);
        send_error("Model exceeds maximum size\0");
      }
      state = RecvModel;
      break;
    case RecvModel:
      memcpy(model_content + model_received_bytes, data, size);
      model_received_bytes += size;
      if (model_received_bytes > model_size) {
        send_error("Too many bytes received for model\0");
      }
      xscope_int(RECV_ACK, 0);
      break;
    case SetTensor:
      tensor_received_bytes = 0;
      sscanf(data, "SET_TENSOR %d %d", &tensor_index, &tensor_size);
      LOG_STATUS("SET_TENSOR index=%d   size=%d\n", tensor_index, tensor_size);
      state = RecvTensor;
      break;
    case RecvTensor:
      model_runner_get_tensor_bytes(tensor_index, &tensor_buffer, &tensor_size);
      memcpy(tensor_buffer + tensor_received_bytes, data, size);
      tensor_received_bytes += size;
      if (tensor_received_bytes > tensor_size) {
        LOG_ERROR("Tensor exceeds size of %d bytes\n", tensor_size);
        send_error("Too many bytes received for tensor\0");
      }
      xscope_int(RECV_ACK, 0);
      break;
    case Initialize:
      status = model_runner_init(model_content, tensor_arena, TENSOR_ARENA_SIZE,
                                 &input_buffer, &input_size, &output_buffer,
                                 &output_size);
      if (status == kTfLiteError) {
        send_error(
            "Unable to initialize inference engine. Check tensor arena "
            "size.\0");
      }
      xscope_int(INIT_ACK, 0);
      break;
    case Invoke:
      status = model_runner_invoke();
      if (status == kTfLiteError) {
        send_error("Unable to invoke inference engine.\0");
      }
      xscope_int(INVOKE_ACK, 0);
      break;
    case GetTensor:
      sscanf(data, "GET_TENSOR %d", &tensor_index);
      model_runner_get_tensor_bytes(tensor_index, &tensor_buffer, &tensor_size);
      LOG_STATUS("GET_TENSOR index=%d  size=%d\n", tensor_index, tensor_size);
      send_tensor(tensor_buffer, tensor_size);
      break;
    case GetProfilerTimes:
      model_runner_get_profiler_times(&profiler_times_count, &profiler_times);
      LOG_STATUS("GET_PROFILER_TIMES profiler_times_count=%d\n",
                 profiler_times_count);
      send_profiler_times(profiler_times_count, profiler_times);
      break;
  }
}