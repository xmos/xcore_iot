// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "inference_engine.h"

#include <platform.h> // for PLATFORM_REFERENCE_MHZ

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "vww.h"
#include "xcore_device_memory.h"

tflite::ErrorReporter *reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::micro::xcore::XCoreInterpreter *interpreter = nullptr;
tflite::micro::xcore::XCoreProfiler *profiler = nullptr;
constexpr int kTensorArenaSize = 100000;
uint8_t tensor_arena[kTensorArenaSize];

void invoke()
{
  // Run inference, and report any error
  printf("Running inference...\n");
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(reporter, "Invoke failed\n");
  }
}

void initialize(unsigned char **input, int *input_size, unsigned char **output,
                int *output_size)
{
  // Set up logging
  static tflite::MicroErrorReporter error_reporter;
  reporter = &error_reporter;
  // Set up profiling.
  static tflite::micro::xcore::XCoreProfiler xcore_profiler;
  profiler = &xcore_profiler;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(vww_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  static tflite::MicroMutableOpResolver<7> resolver;
  resolver.AddSoftmax();
  resolver.AddPad();
  resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode,
                     tflite::ops::micro::xcore::Register_Conv2D_Shallow());
  resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Depthwise_OpCode,
                     tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
  resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_1x1_OpCode,
                     tflite::ops::micro::xcore::Register_Conv2D_1x1());
  resolver.AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode,
                     tflite::ops::micro::xcore::Register_AvgPool2D());
  resolver.AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode,
                     tflite::ops::micro::xcore::Register_FullyConnected_8());

  // Build an interpreter to run the model with
  static tflite::micro::xcore::XCoreInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, reporter, profiler);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(reporter, "AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  *input = (unsigned char *)(interpreter->input(0)->data.raw);
  *input_size = interpreter->input(0)->bytes;
  *output = (unsigned char *)(interpreter->output(0)->data.raw);
  *output_size = interpreter->output(0)->bytes;
}

void print_profiler_summary()
{
  uint32_t count = 0;
  uint32_t total = 0;
  uint32_t time_us = 0;
  const uint32_t *durations = nullptr;
  void *v_resolver = nullptr;
  const char *op_name;

  if (profiler)
  {
    count = profiler->GetNumEvents();
    durations = profiler->GetEventDurations();

    size_t subgraph_idx = 0;
    const tflite::SubGraph *subgraph = model->subgraphs()->Get(subgraph_idx);
    auto *opcodes = model->operator_codes();
    uint32_t operators_size = NumSubgraphOperators(subgraph);
    const tflite::OpResolver *c_resolver = static_cast<const tflite::OpResolver *>(v_resolver);

    for (size_t i = 0; i < operators_size; ++i)
    {
      if (i < count)
      {
        const auto *op = subgraph->operators()->Get(i);
        const size_t index = op->opcode_index();
        const auto *opcode = opcodes->Get(index);
        const TfLiteRegistration *registration = nullptr;

        GetRegistrationFromOpCode(opcode, *c_resolver, reporter,
                                  &registration);

        if (registration->builtin_code == tflite::BuiltinOperator_CUSTOM)
        {
          op_name = registration->custom_name;
        }
        else
        {
          op_name = tflite::EnumNameBuiltinOperator(
              tflite::BuiltinOperator(registration->builtin_code));
        }
        time_us = durations[i] / PLATFORM_REFERENCE_MHZ;
        total += time_us;
        printf("Operator %d, %s took %lu microseconds\n", i, op_name, time_us);
      }
    }
    printf("TOTAL %lu microseconds\n", total);
  }
}