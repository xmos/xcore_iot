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

static char interpreter_buffer[sizeof(tflite::micro::xcore::XCoreInterpreter)];

ModelRunnerStatus cifar10_create(model_runner_t *ctx, const uint8_t* model_content)
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

#ifndef NDEBUG
  if (profiler == nullptr) {
    // Set up profiling
    static profiler_t profiler_s;

    profiler = &profiler_s;
  }
#endif

  return model_runner_create(
      ctx, static_cast<void *>(resolver), static_cast<void *>(profiler),
      static_cast<void *>(interpreter_buffer), model_content);
}

ModelRunnerStatus cifar10_invoke(model_runner_t *ctx) {
  if (profiler) {
    profiler->Reset();
  }

  return model_runner_invoke(ctx);
}


#ifndef NDEBUG

void cifar10_get_profiler_times(model_runner_t *ctx, uint32_t *count,
                                     const uint32_t **times) {
  if (profiler) {
    *count = profiler->GetNumTimes();
    *times = profiler->GetTimes();
  }
}

void cifar10_print_profiler_summary(model_runner_t *ctx) {
  uint32_t count = 0;
  const uint32_t *times = nullptr;

  cifar10_get_profiler_times(ctx, &count, &times);

  model_runner_print_profiler_summary(ctx, count, times);
}

#endif
