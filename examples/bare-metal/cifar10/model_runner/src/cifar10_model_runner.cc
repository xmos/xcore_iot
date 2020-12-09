// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#include "cifar10_model_runner.h"

#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"

typedef tflite::MicroMutableOpResolver<6> resolver_t;

static resolver_t resolver_s;
static resolver_t *resolver = nullptr;

ModelRunnerStatus cifar10_model_runner_create(model_runner_t *ctx, const uint8_t* model_content)
{
  // Set up op resolver
  //   This pulls in all the operation implementations we need.
  if (resolver == nullptr) {
    resolver = &resolver_s;
    resolver->AddSoftmax();
    resolver->AddPad();
    resolver->AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode, tflite::ops::micro::xcore::Register_FullyConnected_8());
    resolver->AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode, tflite::ops::micro::xcore::Register_MaxPool2D());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Shallow());
  }

  return model_runner_create(ctx, static_cast<void *>(resolver), model_content);
}