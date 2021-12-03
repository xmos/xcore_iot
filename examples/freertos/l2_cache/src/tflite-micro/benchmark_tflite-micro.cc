// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcore/hwtimer.h>



#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "benchmark_tflite-micro.h"
#include "print_info.h"

tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;

constexpr int kTensorArenaSize = 45000;
uint8_t tensor_arena[kTensorArenaSize];

extern const unsigned char cifar10_quant_data[];
extern const int cifar10_quant_data_len;

void setup() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(cifar10_quant_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::MicroMutableOpResolver<5>  resolver;
  resolver.AddConv2D();
  resolver.AddMaxPool2D();
  resolver.AddReshape();
  resolver.AddFullyConnected();
  resolver.AddSoftmax();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
}

int run_inference() {
  memset(interpreter->input(0)->data.raw, 0, interpreter->input(0)->bytes);
  interpreter->Invoke();

  return interpreter->output(0)->data.int8[0];
}

#pragma stackfunction 2000
void benchmark_tflu() {
  uint32_t elapsed_time;
  int retval;
  static bool is_setup = false;

  if (!is_setup) {
    setup();
    is_setup = true;
  }

  debug_printf("\nCIFAR-10 model inference\n");
  elapsed_time = get_reference_time();
  retval = run_inference();
  assert(retval == -84);
  elapsed_time = get_reference_time() - elapsed_time;

  print_info(elapsed_time);
}
