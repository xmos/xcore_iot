// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "inference_engine.h"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/version.h"
#include "test_ops_resolver.h"

tflite::TestOpsResolver *resolver = nullptr;
tflite::ErrorReporter *reporter = nullptr;
tflite::Profiler *profiler = nullptr;
tflite::micro::xcore::XCoreInterpreter *interpreter = nullptr;
const tflite::Model *model = nullptr;

void get_tensor_bytes(int index, void **bytes, size_t *size) {
  TfLiteTensor *tensor = interpreter->tensor(index);
  if (tensor != nullptr) {
    *bytes = tensor->data.raw;
    *size = tensor->bytes;
  } else {
    *bytes = nullptr;
    *size = 0;
  }
}

TfLiteStatus invoke_inference_engine() {
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    printf("Invoke failed\n");
  }
  return invoke_status;
}

void reset_inference_engine(uint8_t *tensor_arena, size_t tensor_arena_size) {
  if (interpreter) {
    // delete existing interpreter
    delete interpreter;  // NOTE: interpreter must be deleted before
                         // resolver and reporter
    if (resolver) delete resolver;
    if (reporter) delete reporter;
    // need to memset the arena to 0
    memset(tensor_arena, 0, tensor_arena_size);
  }
}

TfLiteStatus initialize_inference_engine(uint8_t *model_content,
                                         uint8_t *tensor_arena,
                                         size_t tensor_arena_size,
                                         uint8_t **input, size_t *input_size,
                                         uint8_t **output,
                                         size_t *output_size) {
  // Set up logging
  reporter = new tflite::MicroErrorReporter();

  //   // Set up profiling.
  //   profiler = new xcore::XCoreProfiler xcore_profiler(reporter);

  // Create ops resolver
  resolver = new tflite::TestOpsResolver();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf(
        "Model provided is schema version %lu not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return kTfLiteError;
  }

  // Build an interpreter to run the model with
  interpreter = new tflite::micro::xcore::XCoreInterpreter(
      model, *resolver, tensor_arena, tensor_arena_size, reporter, true,
      profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    printf("AllocateTensors() failed");
    return kTfLiteError;
  }

  // Obtain pointers to the model's input and output tensors.
  *input = (unsigned char *)(interpreter->input(0)->data.raw);
  *input_size = interpreter->input(0)->bytes;
  *output = (unsigned char *)(interpreter->output(0)->data.raw);
  *output_size = interpreter->output(0)->bytes;

  return kTfLiteOk;
}
