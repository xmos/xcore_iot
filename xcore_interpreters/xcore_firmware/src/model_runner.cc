// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "model_runner.h"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/version.h"
#include "test_ops_resolver.h"

// shorthand typedefs
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;
typedef tflite::micro::xcore::XCoreProfiler profiler_t;
typedef tflite::TestOpsResolver resolver_t;
typedef tflite::Model model_t;

// static variables
static error_reporter_t error_reporter_s;
static error_reporter_t *reporter = nullptr;
static resolver_t resolver_s;
static resolver_t *resolver = nullptr;
static profiler_t profiler_s;
static profiler_t *profiler = nullptr;
static interpreter_t *interpreter = nullptr;
static const model_t *model = nullptr;

// static buffer for interpreter_t class allocation
uint8_t interpreter_buffer[sizeof(interpreter_t)];

void model_runner_tensor_bytes_get(int index, void **bytes, size_t *size) {
  TfLiteTensor *tensor = interpreter->tensor(index);
  if (tensor != nullptr) {
    *bytes = tensor->data.raw;
    *size = tensor->bytes;
  } else {
    *bytes = nullptr;
    *size = 0;
  }
}

ModelRunnerStatus model_runner_invoke() {
  // reset the profiler
  if (profiler) {
    profiler->Reset();
  }

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    return InvokeError;
  }
  return Ok;
}

void model_runner_reset(uint8_t *tensor_arena, size_t tensor_arena_size) {
  if (interpreter) {
    // delete existing interpreter
    delete interpreter;  // NOTE: interpreter must be deleted before
                         // resolver and reporter
    // need to memset the arena to 0
    memset(tensor_arena, 0, tensor_arena_size);
  }
}

ModelRunnerStatus model_runner_init(uint8_t *model_content,
                                    uint8_t *tensor_arena,
                                    size_t tensor_arena_size, uint8_t **input,
                                    size_t *input_size, uint8_t **output,
                                    size_t *output_size) {
  // Set up logging
  if (reporter == nullptr) {
    reporter = &error_reporter_s;
  }

#ifndef NDEBUG
  // Set up profiling
  if (profiler == nullptr) {
    profiler = &profiler_s;
  }
#endif

  // Set up op resolver
  //   This pulls in all the operation implementations we need.
  if (resolver == nullptr) {
    resolver = &resolver_s;
  }

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return ModelVersionError;
  }

  // Build an interpreter to run the model with
  interpreter = new (interpreter_buffer)
      interpreter_t(model, *resolver, tensor_arena, tensor_arena_size, reporter,
                    true, profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    return AllocateTensorsError;
  }

  // Obtain pointers to the model's input and output tensors.
  *input = reinterpret_cast<unsigned char *>(interpreter->input(0)->data.raw);
  *input_size = interpreter->input(0)->bytes;
  *output = reinterpret_cast<unsigned char *>(interpreter->output(0)->data.raw);
  *output_size = interpreter->output(0)->bytes;

  return Ok;
}

void model_runner_profiler_times_get(uint32_t *count, const uint32_t **times) {
  if (profiler) {
    *count = profiler->GetNumTimes();
    *times = profiler->GetTimes();
  } else {
    *count = 0;
    *times = static_cast<uint32_t *>(nullptr);
  }
}