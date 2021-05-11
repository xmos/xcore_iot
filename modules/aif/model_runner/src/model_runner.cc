// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "model_runner.h"

#include <ctime>

#include <platform.h>  // for PLATFORM_REFERENCE_MHZ
#include <xcore/assert.h>

#include "model_memory_loader.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

// typedefs
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::SimpleMemoryAllocator simple_allocator_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::MicroOpResolver micro_op_resolver_t;
typedef tflite::MicroProfiler tflite_profiler_t;
typedef tflite::micro::xcore::GenericDispatcher dispatcher_t;
typedef tflite::micro::xcore::ModelMemoryLoader memory_loader_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;

// static variables
static error_reporter_t error_reporter_s;
static dispatcher_t dispatcher_s;
static memory_loader_t memory_loader_s;
static error_reporter_t *reporter = nullptr;

static micro_allocator_t *allocator = nullptr;

size_t model_runner_buffer_size_get() { return sizeof(interpreter_t); }

void model_runner_init(uint8_t *arena, size_t arena_size) {
  xassert(arena);
  xassert(arena_size > 0);

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

ModelRunnerStatus model_runner_allocate(model_runner_t *ctx,
                                        const uint8_t *model_content) {
  xassert(model_content);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model *model;
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return ModelVersionError;
  }

  // Get model specific resolver
  void *v_resolver = nullptr;
  ctx->resolver_get_fun(&v_resolver);
  micro_op_resolver_t *resolver =
      static_cast<micro_op_resolver_t *>(v_resolver);

  // Get model specific profiler
  void *v_profiler = nullptr;
  ctx->profiler_get_fun(&v_profiler);
  tflite_profiler_t *profiler = static_cast<tflite_profiler_t *>(v_profiler);

  // Allocate buffer for interpreter (if not already allocated)
  if (ctx->hInterpreter == nullptr) {
    ctx->hInterpreter = malloc(model_runner_buffer_size_get());
  }
  // Build an interpreter to run the model with
  interpreter_t *interpreter = new (ctx->hInterpreter)
      interpreter_t(model, *resolver, allocator, reporter, dispatcher_s,
                    memory_loader_s, profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    return AllocateTensorsError;
  }

  return Ok;
}

int8_t *model_runner_input_buffer_get(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  return interpreter->input(0)->data.int8;
}

size_t model_runner_input_size_get(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  return interpreter->input(0)->bytes;
}

void model_runner_input_quant_get(model_runner_t *ctx, float *scale,
                                  int *zero_point) {
  xassert(scale);
  xassert(zero_point);

  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  *scale = interpreter->input(0)->params.scale;
  *zero_point = interpreter->input(0)->params.zero_point;
}

ModelRunnerStatus model_runner_invoke(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);

  // Rset the profiler
  ctx->profiler_reset_fun();

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    return InvokeError;
  }

  return Ok;
}

int8_t *model_runner_output_buffer_get(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  return interpreter->output(0)->data.int8;
}

size_t model_runner_output_size_get(model_runner_t *ctx) {
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  return interpreter->output(0)->bytes;
}

void model_runner_output_quant_get(model_runner_t *ctx, float *scale,
                                   int *zero_point) {
  xassert(scale);
  xassert(zero_point);

  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);
  *scale = interpreter->output(0)->params.scale;
  *zero_point = interpreter->output(0)->params.zero_point;
}

#ifndef NDEBUG

void model_runner_profiler_durations_get(model_runner_t *ctx, uint32_t *count,
                                         const uint32_t **durations) {
  xassert(count);
  xassert(durations);

  ctx->profiler_durations_get_fun(count, durations);
}

void model_runner_profiler_summary_print(model_runner_t *ctx) {
  uint32_t count = 0;
  uint32_t total = 0;
  uint32_t time_us = 0;
  const uint32_t *durations = nullptr;
  const char *op_name;

  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->hInterpreter);

  ctx->profiler_durations_get_fun(&count, &durations);

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
      time_us = durations[i] / PLATFORM_REFERENCE_MHZ;
      total += time_us;
      printf("Operator %d, %s took %lu microseconds\n", i, op_name, time_us);
    }
  }
  printf("TOTAL %lu microseconds\n", total);
}

#endif
