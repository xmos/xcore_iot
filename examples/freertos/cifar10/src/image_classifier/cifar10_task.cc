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

#include "app_conf.h"
#include "cifar10_model_data.h"
#include "cifar10_task.h"
#include "fs_support.h"

void cifar10_task_inputs(void *args);
void cifar10_task_inference(void *args);

}; // extern "C"

typedef struct inference_engine_args {
  QueueHandle_t input_queue;
  QueueHandle_t output_queue;
} inference_engine_args_t;

#define TENSOR_ARENA_SIZE 64000

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

void cifar10_task_inputs(void *args) {
  inference_engine_args_t *targs = (inference_engine_args_t *)args;
  QueueHandle_t input_queue = targs->input_queue;
  QueueHandle_t output_queue = targs->output_queue;
  FIL current_file;
  unsigned int file_size;
  uint8_t *input_tensor = nullptr;
  uint8_t *output_tensor = nullptr;
  FRESULT result;
  unsigned int bytes_read = 0;
  char classification[12] = {0};

  while (1) {
    for (int i = 0;
         i < (sizeof(test_input_files) / sizeof(test_input_files[0])); i++) {
      if (rtos_ff_get_file(test_input_files[i], &current_file, &file_size) ==
          0) {
        rtos_printf("Failed to load file %s\n", test_input_files[i]);
        continue;
      }

      input_tensor = (uint8_t *)pvPortMalloc(sizeof(unsigned char) * file_size);

      configASSERT(input_tensor !=
                   NULL); /* Failed to allocate memory for file data */

      result = f_read(&current_file, input_tensor, file_size, &bytes_read);

      xQueueSend(input_queue, &input_tensor, portMAX_DELAY);
      xQueueReceive(output_queue, &output_tensor, portMAX_DELAY);
      vPortFree(input_tensor);

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
    }
    rtos_printf("All files complete.  Repeating in 5 seconds...\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

#pragma stackfunction 500
void cifar10_task_inference(void *args) {
  inference_engine_args_t *targs = (inference_engine_args_t *)args;
  QueueHandle_t input_queue = targs->input_queue;
  QueueHandle_t output_queue = targs->output_queue;
  int8_t *input_buffer = nullptr;
  size_t input_size = 0;
  int8_t *output_buffer = nullptr;
  size_t output_size = 0;
  uint8_t *tensor_arena = nullptr;
  uint8_t *input_tensor = nullptr;
  // dispatcher_t *dispatcher;
  xcore::rtos::InferenceEngine<7, 18> inference_engine;

  tensor_arena = (uint8_t *)pvPortMalloc(TENSOR_ARENA_SIZE);

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
  resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_V2_OpCode,
                      tflite::ops::micro::xcore::rtos::Register_Conv2D_V2());
  resolver->AddCustom(
      tflite::ops::micro::xcore::Load_Flash_OpCode,
      tflite::ops::micro::xcore::rtos::Register_LoadFromFlash());

  // Load the model
  xcore::rtos::FatFSLoader flash_loader;
  FIL model_file;
  unsigned int model_file_size;
  if (rtos_ff_get_file("model.bin", &model_file, &model_file_size) == 0) {
    rtos_printf("Failed to load file model.bin\n");
    vPortFree(tensor_arena);
    vTaskDelete(NULL);
  }
  flash_loader.Init(&model_file);
  if (inference_engine.LoadModel(cifar10_model_data, &flash_loader) !=
      xcore::rtos::InferenceEngineStatus::Ok) {
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
    xQueueReceive(input_queue, &input_tensor, portMAX_DELAY);
    memcpy(input_buffer, input_tensor, input_size);

    rtos_printf("Running inference...\n");
    inference_engine.Invoke();
    inference_engine.PrintProfilerSummary();

    xQueueSend(output_queue, &output_buffer, portMAX_DELAY);
  }
}

void cifar10_image_classifier_task_create(unsigned priority) {
  inference_engine_args_t *args =
      (inference_engine_args_t *)pvPortMalloc(sizeof(inference_engine_args_t));
  QueueHandle_t input_queue = xQueueCreate(1, sizeof(int32_t *));
  QueueHandle_t output_queue = xQueueCreate(1, sizeof(int32_t *));

  configASSERT(args);
  configASSERT(input_queue);
  configASSERT(output_queue);

  args->input_queue = input_queue;
  args->output_queue = output_queue;

  xTaskCreate((TaskFunction_t)cifar10_task_inference, "cifar10_inference",
              RTOS_THREAD_STACK_SIZE(cifar10_task_inference), args, priority,
              NULL);

  xTaskCreate((TaskFunction_t)cifar10_task_inputs, "cifar10_inputs",
              RTOS_THREAD_STACK_SIZE(cifar10_task_inputs), args, priority,
              NULL);
}
