// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#include "cifar10_model_runner.h"

#include "drivers/sw_services/model_runner/api/model_runner_profiler.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"

typedef tflite::MicroMutableOpResolver<6> resolver_t;
typedef xcore::ModelRunnerProfiler<9> profiler_t;

static resolver_t resolver_s;
static resolver_t *resolver = nullptr;

static profiler_t *profiler = nullptr;

__attribute__((fptrgroup("model_runner_function")))
void cifar10_resolver_get(void **v_resolver)
{
  // Set up op resolver
  //   This pulls in all the operation implementations we need.
  if (resolver == nullptr) {
    resolver = &resolver_s;
    resolver->AddPad();
    resolver->AddSoftmax();
    resolver->AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode, tflite::ops::micro::xcore::Register_MaxPool2D());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver->AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode, tflite::ops::micro::xcore::Register_FullyConnected_8());
  }

  *v_resolver = static_cast<void *>(resolver);
}

__attribute__((fptrgroup("model_runner_function")))
void cifar10_profiler_get(void **v_profiler) {
#ifndef NDEBUG
  if (profiler == nullptr) {
    // Set up profiling
    static profiler_t profiler_s;

    profiler = &profiler_s;
  }
#endif

  *v_profiler = static_cast<void *>(profiler);
}

__attribute__((fptrgroup("model_runner_function")))
void cifar10_profiler_reset() {
  if (profiler) {
    profiler->Reset();
  }
}

#ifndef NDEBUG

__attribute__((fptrgroup("model_runner_function")))
void cifar10_profiler_times_get(uint32_t *count, const uint32_t **times) {
  if (profiler) {
    *count = profiler->GetNumTimes();
    *times = profiler->GetTimes();
  }
}

#endif

//********************************
// Create a cifar10 model runner.
//********************************
void cifar10_model_runner_create(model_runner_t *ctx, void *buffer) {
  ctx->hInterpreter = buffer;
  ctx->resolver_get_fun = &cifar10_resolver_get;
  ctx->profiler_get_fun = &cifar10_profiler_get;
  ctx->profiler_reset_fun = &cifar10_profiler_reset;
  ctx->profiler_times_get_fun = &cifar10_profiler_times_get;
}