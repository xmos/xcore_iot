// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "inference_engine.h"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

tflite::ErrorReporter *reporter = nullptr;
tflite::Profiler *profiler = nullptr;
const tflite::Model *model = nullptr;
tflite::micro::xcore::XCoreInterpreter *interpreter = nullptr;

// static buffer for XCoreInterpreter class allocation
uint8_t interpreter_buffer[sizeof(tflite::micro::xcore::XCoreInterpreter)];

void invoke() {
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(reporter, "Invoke failed\n");
  }
}

void initialize(const unsigned char *model_content, uint8_t *tensor_arena,
                size_t tensor_arena_size, uint8_t **input, size_t *input_size,
                uint8_t **output, size_t *output_size) {
  // Set up logging
  static tflite::MicroErrorReporter error_reporter;
  if (reporter == nullptr) {
    reporter = &error_reporter;
  }
  // Set up profiling.
  static tflite::micro::xcore::XCoreProfiler xcore_profiler(reporter);
  if (profiler == nullptr) {
    profiler = &xcore_profiler;
  }

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    exit(1);
  }

  // This pulls in all the operation implementations we need.
  static tflite::MicroMutableOpResolver<7> resolver;
  resolver.AddSoftmax();
  resolver.AddPad();
  resolver.AddCustom("XC_maxpool2d",
                     tflite::ops::micro::xcore::Register_MaxPool2D());
  resolver.AddCustom("XC_fc_deepin_anyout",
                     tflite::ops::micro::xcore::Register_FullyConnected_16());
  resolver.AddCustom("XC_conv2d_shallowin",
                     tflite::ops::micro::xcore::Register_Conv2D_Shallow());
  resolver.AddCustom("XC_conv2d_deep",
                     tflite::ops::micro::xcore::Register_Conv2D_Deep());
  resolver.AddCustom("XC_requantize_16_to_8",
                     tflite::ops::micro::xcore::Register_Requantize_16_to_8());

  // Build an interpreter to run the model with
  if (interpreter) {
    // We already have an interpreter so we need to explicitly call the
    // destructor here but NOT delete the object
    interpreter->~XCoreInterpreter();
  }
  interpreter = new (interpreter_buffer) tflite::micro::xcore::XCoreInterpreter(
      model, resolver, tensor_arena, tensor_arena_size, reporter, true,
      profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(reporter, "AllocateTensors() failed");
    exit(1);
  }

  // Obtain pointers to the model's input and output tensors.
  *input = (unsigned char *)(interpreter->input(0)->data.raw);
  *input_size = interpreter->input(0)->bytes;
  *output = (unsigned char *)(interpreter->output(0)->data.raw);
  *output_size = interpreter->output(0)->bytes;
}