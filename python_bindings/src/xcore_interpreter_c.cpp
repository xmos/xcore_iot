// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_extended_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"
#include "tensorflow/lite/micro/recording_micro_allocator.h"
#include "tensorflow/lite/version.h"

//*****************************************
//*****************************************
//*****************************************
// C-API callable from Python
//*****************************************
//*****************************************
//*****************************************

typedef struct ExtendedXCoreInterpreterContext {
  tflite::micro::xcore::BufferedErrorReporter* reporter;
  tflite::AllOpsResolver* resolver;
  const tflite::Model* model;
  char* model_buffer;
  uint8_t* tensor_arena;
  size_t tensor_arena_size;
  tflite::RecordingMicroAllocator* allocator;
  tflite::micro::xcore::ExtendedXCoreInterpreter* interpreter;
} ExtendedXCoreInterpreterState;

extern "C" {
ExtendedXCoreInterpreterContext* new_interpreter() {
  ExtendedXCoreInterpreterContext* ctx = new ExtendedXCoreInterpreterContext();
  ctx->reporter = nullptr;
  ctx->resolver = nullptr;
  ctx->model = nullptr;
  ctx->model_buffer = nullptr;
  ctx->tensor_arena = nullptr;
  ctx->allocator = nullptr;  // NOTE: the allocator is created in the arena so
                             // it does not need to be deleted
  ctx->interpreter = nullptr;
  return ctx;
}

void delete_interpreter(ExtendedXCoreInterpreterContext* ctx) {
  if (ctx->interpreter)
    delete ctx->interpreter;  // NOTE: interpreter must be deleted before
                              // resolver, reporter, tensor_arena
                              // and model_buffer
  if (ctx->resolver) delete ctx->resolver;
  if (ctx->reporter) delete ctx->reporter;
  if (ctx->tensor_arena) delete[] ctx->tensor_arena;
  if (ctx->model_buffer) delete[] ctx->model_buffer;
  delete ctx;
}

int initialize(ExtendedXCoreInterpreterContext* ctx, const char* model_content,
               size_t model_content_size, size_t tensor_arena_size) {
  // We need to keep a copy of the model content
  ctx->model_buffer = new char[model_content_size];
  memcpy(ctx->model_buffer, model_content, model_content_size);

  // Create error reporter
  ctx->reporter = new tflite::micro::xcore::BufferedErrorReporter();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  ctx->model = tflite::GetModel(ctx->model_buffer);
  if (ctx->model->version() != TFLITE_SCHEMA_VERSION) {
    ctx->reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        ctx->model->version(), TFLITE_SCHEMA_VERSION);
    return kTfLiteError;
  }

  // Create all ops resolver and add xCORE custom operators
  ctx->resolver = new tflite::AllOpsResolver();
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode,
                           tflite::ops::micro::xcore::Register_MaxPool2D());
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode,
                           tflite::ops::micro::xcore::Register_AvgPool2D());
  ctx->resolver->AddCustom(
      tflite::ops::micro::xcore::AvgPool2D_Global_OpCode,
      tflite::ops::micro::xcore::Register_AvgPool2D_Global());
  ctx->resolver->AddCustom(
      tflite::ops::micro::xcore::FullyConnected_8_OpCode,
      tflite::ops::micro::xcore::Register_FullyConnected_8());
  ctx->resolver->AddCustom(
      tflite::ops::micro::xcore::Conv2D_Shallow_OpCode,
      tflite::ops::micro::xcore::Register_Conv2D_Shallow());
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode,
                           tflite::ops::micro::xcore::Register_Conv2D_Deep());
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_1x1_OpCode,
                           tflite::ops::micro::xcore::Register_Conv2D_1x1());
  ctx->resolver->AddCustom(
      tflite::ops::micro::xcore::Conv2D_Depthwise_OpCode,
      tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::Lookup_8_OpCode,
                           tflite::ops::micro::xcore::Register_Lookup_8());
  ctx->resolver->AddCustom(tflite::ops::micro::xcore::Bsign_8_OpCode,
                           tflite::ops::micro::xcore::Register_BSign_8());

  ctx->tensor_arena = new uint8_t[tensor_arena_size];
  memset(ctx->tensor_arena, 0, tensor_arena_size);

  // Create recording allocator
  ctx->allocator = tflite::RecordingMicroAllocator::Create(
      ctx->tensor_arena, tensor_arena_size, ctx->reporter);

  // Build an interpreter to run the model with.
  ctx->interpreter = new tflite::micro::xcore::ExtendedXCoreInterpreter(
      ctx->model, *ctx->resolver, ctx->allocator,
      reinterpret_cast<tflite::ErrorReporter*>(ctx->reporter));

  return kTfLiteOk;
}

int allocate_tensors(ExtendedXCoreInterpreterContext* ctx) {
  return ctx->interpreter->AllocateTensors();
}

int tensors_size(ExtendedXCoreInterpreterContext* ctx) {
  return ctx->interpreter->tensors_size();
}

size_t inputs_size(ExtendedXCoreInterpreterContext* ctx) {
  return ctx->interpreter->inputs_size();
}

size_t outputs_size(ExtendedXCoreInterpreterContext* ctx) {
  return ctx->interpreter->outputs_size();
}

size_t arena_used_bytes(ExtendedXCoreInterpreterContext* ctx) {
  return ctx->interpreter->arena_used_bytes();
}

int set_tensor(ExtendedXCoreInterpreterContext* ctx, size_t tensor_index,
               const void* value, const int size, const int* shape,
               const int type) {
  return ctx->interpreter->SetTensor(tensor_index, value, size, shape, type);
}

int get_tensor(ExtendedXCoreInterpreterContext* ctx, size_t tensor_index,
               void* value, const int size, const int* shape, const int type) {
  return ctx->interpreter->GetTensor(tensor_index, value, size, shape, type);
}

size_t get_tensor_details_buffer_sizes(ExtendedXCoreInterpreterContext* ctx,
                                       size_t tensor_index, size_t* dims,
                                       size_t* scales, size_t* zero_points) {
  return ctx->interpreter->GetTensorDetailsBufferSizes(tensor_index, dims,
                                                       scales, zero_points);
}

int get_tensor_details(ExtendedXCoreInterpreterContext* ctx,
                       size_t tensor_index, char* name, int name_len,
                       int* shape, int* type, float* scale, int* zero_point) {
  return ctx->interpreter->GetTensorDetails(tensor_index, name, name_len, shape,
                                            type, scale, zero_point);
}

size_t get_operator_details_buffer_sizes(ExtendedXCoreInterpreterContext* ctx,
                                         size_t operator_index, size_t* inputs,
                                         size_t* outputs) {
  return ctx->interpreter->GetOperatorDetailsBufferSizes(operator_index, inputs,
                                                         outputs);
}

int get_operator_details(ExtendedXCoreInterpreterContext* ctx,
                         size_t operator_index, char* name, int name_len,
                         int* version, int* inputs, int* outputs) {
  return ctx->interpreter->GetOperatorDetails(operator_index, name, name_len,
                                              version, inputs, outputs);
}

size_t input_tensor_index(ExtendedXCoreInterpreterContext* ctx,
                          size_t input_index) {
  return ctx->interpreter->input_tensor_index(input_index);
}

size_t output_tensor_index(ExtendedXCoreInterpreterContext* ctx,
                           size_t output_index) {
  return ctx->interpreter->output_tensor_index(output_index);
}

int invoke(
    ExtendedXCoreInterpreterContext* ctx,
    tflite::micro::xcore::invoke_callback_t preinvoke_callback = nullptr,
    tflite::micro::xcore::invoke_callback_t postinvoke_callback = nullptr) {
  return ctx->interpreter->Invoke(preinvoke_callback, postinvoke_callback);
}

size_t get_allocations(ExtendedXCoreInterpreterContext* ctx, char* msg) {
  ctx->reporter->Clear();
  ctx->allocator->PrintAllocations();
  const std::string& alloc_msg = ctx->reporter->GetError();
  std::strncpy(msg, alloc_msg.c_str(), alloc_msg.length());

  return alloc_msg.length();
}

size_t get_error(ExtendedXCoreInterpreterContext* ctx, char* msg) {
  const std::string& error_msg = ctx->reporter->GetError();
  std::strncpy(msg, error_msg.c_str(), error_msg.length());
  return error_msg.length();
}

}  // extern "C"
