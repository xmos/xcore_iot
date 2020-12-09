// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "drivers/sw_services/model_runner/api/model_runner.h"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

// typedefs
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::SimpleMemoryAllocator simple_allocator_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::MicroOpResolver micro_op_resolver_t;
typedef tflite::Profiler tflite_profiler_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;

// static variables
static error_reporter_t error_reporter_s;
static error_reporter_t *reporter = nullptr;

static micro_allocator_t *allocator = nullptr;

void model_runner_init(uint8_t *arena, int arena_size) {
  // Set up error reporting
  if (reporter == nullptr) {
    reporter = &error_reporter_s;
  }

  // Set up allocator
  static simple_allocator_t simple_allocator_s(reporter, arena, arena_size);
  if (allocator == nullptr) {
    allocator = micro_allocator_t::Create(&simple_allocator_s, reporter);
  }
}

ModelRunnerStatus model_runner_create(model_runner_t *ctx, void *resolver,
                                      void *profiler, void *buffer,
                                      const uint8_t *model_content) {
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model *model;
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return ModelVersionError;
  }

  micro_op_resolver_t *op_resolver =
      static_cast<micro_op_resolver_t *>(resolver);

  tflite_profiler_t *op_profiler = static_cast<tflite_profiler_t *>(profiler);

  // Build an interpreter to run the model with
  interpreter_t *interpreter = new (buffer) interpreter_t(
      model, *op_resolver, allocator, reporter, true, op_profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    return AllocateTensorsError;
  }

  ctx->handle = static_cast<void *>(interpreter);

  return Ok;
}

int8_t *model_runner_get_input(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->input(0)->data.int8;
}

size_t model_runner_get_input_size(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->input(0)->bytes;
}

void model_runner_get_input_quant(model_runner_t *ctx, float *scale,
                                  int *zero_point) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  *scale = interpreter->input(0)->params.scale;
  *zero_point = interpreter->input(0)->params.zero_point;
}

ModelRunnerStatus model_runner_invoke(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    return InvokeError;
  }

  return Ok;
}

int8_t *model_runner_get_output(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->output(0)->data.int8;
}

size_t model_runner_get_output_size(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->output(0)->bytes;
}

void model_runner_get_output_quant(model_runner_t *ctx, float *scale,
                                   int *zero_point) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  *scale = interpreter->output(0)->params.scale;
  *zero_point = interpreter->output(0)->params.zero_point;
}

#ifndef NDEBUG

void model_runner_print_profiler_summary(model_runner_t *ctx, uint32_t count,
                                         const uint32_t *times) {
  const char *op_name;
  uint32_t total = 0;

  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);

  for (size_t i = 0; i < interpreter->operators_size(); ++i) {
    if (i < count) {
      tflite::NodeAndRegistration node_and_reg =
          interpreter->node_and_registration(static_cast<int>(i));
      const TfLiteRegistration *registration = node_and_reg.registration;
      if (registration->builtin_code == tflite::BuiltinOperator_CUSTOM) {
        op_name = registration->custom_name;
      } else {
        op_name = tflite::EnumNameBuiltinOperator(
            tflite::BuiltinOperator(registration->builtin_code));
      }
      total += times[i];
      printf("Operator %d, %s took %lu microseconds\n", i, op_name, times[i]);
    }
  }
  printf("TOTAL %lu microseconds\n", total);
}

#endif