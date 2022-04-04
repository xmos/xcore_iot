// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "InferenceEngine.hpp"

extern "C" {

#include <platform.h>
#include <xs1.h>

#include "fs_support.h"
#include "app_conf.h"
#include "cifar10_task.h"
#include "cifar10_model_data.h"

void cifar10_task_app(void *args);
void cifar10_runner_rx(void *args);

}; // extern "C"

typedef struct inference_engine_args {
  QueueHandle_t input_queue;
  rtos_intertile_address_t *intertile_addr;
} inference_engine_args_t;

#define TENSOR_ARENA_SIZE 58000

static int argmax(const int8_t *A, const int N) {
  int m = 0;

  for (int i = 1; i < N; i++) {
    if (A[i] > A[m]) {
      m = i;
    }
  }

  return m;
}

static const char *test_input_files[] = {
    "airplane.bin", "bird.bin",  "cat.bin",  "deer.bin",
    "frog.bin",     "horse.bin", "truck.bin"};

void cifar10_task_app(void *args) {
  rtos_intertile_address_t *adr = (rtos_intertile_address_t *)args;
  FIL current_file;
  unsigned int file_size;
  uint8_t *data = NULL;
  FRESULT result;
  unsigned int bytes_read = 0;
  uint8_t *output_tensor;
  int output_tensor_len;
  char classification[12] = {0};

  while (1) {
    for (int i = 0;
         i < (sizeof(test_input_files) / sizeof(test_input_files[0])); i++) {
      if (rtos_ff_get_file(test_input_files[i], &current_file, &file_size) ==
          0) {
        rtos_printf("Failed to load file %s\n", test_input_files[i]);
        continue;
      }

      data = (uint8_t *) pvPortMalloc(sizeof(unsigned char) * file_size);

      configASSERT(data != NULL); /* Failed to allocate memory for file data */

      result = f_read(&current_file, data, file_size, &bytes_read);

      rtos_intertile_tx(adr->intertile_ctx, adr->port, data, file_size);

      vPortFree(data);

      output_tensor_len =
          rtos_intertile_rx(adr->intertile_ctx, adr->port,
                            (void **)&output_tensor, portMAX_DELAY);

      switch (argmax((int8_t *)output_tensor, 10)) {
      case 0:
        rtos_snprintf(classification, 9, "Airplane");
        break;
      case 1:
        rtos_snprintf(classification, 11, "Automobile");
        break;
      case 2:
        rtos_snprintf(classification, 5, "Bird");
        break;
      case 3:
        rtos_snprintf(classification, 4, "Cat");
        break;
      case 4:
        rtos_snprintf(classification, 5, "Deer");
        break;
      case 5:
        rtos_snprintf(classification, 4, "Dog");
        break;
      case 6:
        rtos_snprintf(classification, 5, "Frog");
        break;
      case 7:
        rtos_snprintf(classification, 6, "Horse");
        break;
      case 8:
        rtos_snprintf(classification, 5, "Ship");
        break;
      case 9:
        rtos_snprintf(classification, 6, "Truck");
        break;
      default:
        break;
      }
      rtos_printf("Classification of file %s is %s\n", test_input_files[i],
                  classification);
      vPortFree(output_tensor);
    }
    rtos_printf("All files complete.  Repeating in 5 seconds...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void cifar10_runner_rx(void *args) {
  inference_engine_args_t *targs = (inference_engine_args_t *)args;
  QueueHandle_t q = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  uint8_t *input_tensor;
  int input_tensor_len;

  while (1) {
    input_tensor_len = rtos_intertile_rx(adr->intertile_ctx, adr->port,
                                         (void **)&input_tensor, portMAX_DELAY);
    xQueueSend(q, &input_tensor, portMAX_DELAY);
  }
}

static void cifar10_task_runner(void *args) {
  inference_engine_args_t *targs = (inference_engine_args_t *)args;
  QueueHandle_t q = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  int8_t *input_buffer = NULL;
  size_t input_size = 0;
  int8_t *output_buffer = NULL;
  size_t output_size = 0;
  uint8_t *tensor_arena = NULL;
  uint8_t *input_tensor;
  // dispatcher_t *dispatcher;
  xcore::RTOSInferenceEngine<6, 8> inference_engine;

  tensor_arena = (uint8_t *) pvPortMalloc(TENSOR_ARENA_SIZE);

  // dispatcher = dispatcher_create();
  // dispatcher_thread_init(dispatcher, appconfDISPATCHER_LENGTH,
  //                        appconfDISPATCHER_THREAD_COUNT,
  //                        appconfDISPATCHER_THREAD_PRIORITY);

  auto resolver = inference_engine.Initialize(tensor_arena, TENSOR_ARENA_SIZE);

  // Register the model operators
  resolver->AddSoftmax();
  resolver->AddPad();
  resolver->AddConv2D();
  resolver->AddReshape();
  resolver->AddMaxPool2D();
  resolver->AddCustom(tflite::ops::micro::xcore::rtos::Conv2D_V2_OpCode,
                      tflite::ops::micro::xcore::rtos::Register_Conv2D_V2());

  // Load the model
  if (inference_engine.LoadModel(cifar10_model_data) !=  xcore::InferenceEngineStatus::Ok) {
    rtos_printf("Invalid model provided!\n");
    vPortFree(tensor_arena);
    vTaskDelete(NULL);
  }

  input_buffer = inference_engine.GetInputBuffer();
  input_size = inference_engine.GetInputSize();
  output_buffer = inference_engine.GetOutputBuffer();
  output_size = inference_engine.GetOutputSize();

  while (1) {
    rtos_printf("Wait for input tensor...\n");
    xQueueReceive(q, &input_tensor, portMAX_DELAY);

    memcpy(input_buffer, input_tensor, input_size);
    vPortFree(input_tensor);

    rtos_printf("Running inference...\n");
    inference_engine.Invoke();
    inference_engine.PrintProfilerSummary();

    rtos_intertile_tx(adr->intertile_ctx, adr->port, output_buffer,
                      output_size);
  }
}

void cifar10_app_task_create(rtos_intertile_address_t *intertile_addr,
                             unsigned priority) {
  xTaskCreate((TaskFunction_t)cifar10_task_app, "cifar10",
              RTOS_THREAD_STACK_SIZE(cifar10_task_app), intertile_addr,
              priority, NULL);
}

void cifar10_image_classifier_task_create(rtos_intertile_address_t *intertile_addr,
                                      unsigned priority) {
  inference_engine_args_t *args = (inference_engine_args_t *) pvPortMalloc(sizeof(inference_engine_args_t));
  QueueHandle_t input_queue = xQueueCreate(1, sizeof(int32_t *));

  configASSERT(args);
  configASSERT(input_queue);

  args->input_queue = input_queue;
  args->intertile_addr = intertile_addr;

  xTaskCreate((TaskFunction_t)cifar10_task_runner, "cifar10", 500, args,
              priority, NULL);

  xTaskCreate((TaskFunction_t)cifar10_runner_rx, "cifar10_rx",
              RTOS_THREAD_STACK_SIZE(cifar10_runner_rx), args, priority - 1,
              NULL);
}

