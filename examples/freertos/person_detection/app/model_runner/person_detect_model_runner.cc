// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#include "person_detect_model_runner.h"

#include "model_runner_profiler.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"

typedef tflite::MicroMutableOpResolver<7> resolver_t;
typedef xcore::ModelRunnerProfiler<31> profiler_t;

static resolver_t resolver_s;
static resolver_t *resolver = nullptr;

static profiler_t *profiler = nullptr;

__attribute__((fptrgroup("model_runner_resolver_get_fptr_grp")))
void person_detect_resolver_get(void **v_resolver)
{
  // Set up op resolver
  //   This pulls in all the operation implementations we need.
  if (resolver == nullptr) {
    resolver = &resolver_s;
    resolver->AddPad();
    resolver->AddSoftmax();
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Depthwise_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
    resolver->AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode, tflite::ops::micro::xcore::Register_FullyConnected_8());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode, tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_1x1_OpCode, tflite::ops::micro::xcore::Register_Conv2D_1x1());
    resolver->AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode, tflite::ops::micro::xcore::Register_AvgPool2D());
  }

  *v_resolver = static_cast<void *>(resolver);
}

#ifndef NDEBUG

__attribute__((fptrgroup("model_runner_profiler_get_fptr_grp")))
void person_detect_profiler_get(void **v_profiler) {
  if (profiler == nullptr) {
    // Set up profiling
    static profiler_t profiler_s;

    profiler = &profiler_s;
  }

  *v_profiler = static_cast<void *>(profiler);
}

__attribute__((fptrgroup("model_runner_profiler_reset_fptr_grp")))
void person_detect_profiler_reset() {
  if (profiler) {
    profiler->ClearEvents();
  }
}

__attribute__((fptrgroup("model_runner_profiler_durations_get_fptr_grp")))
void person_detect_profiler_durations_get(uint32_t *count, const uint32_t **durations) {
  if (profiler) {
    *count = profiler->GetNumEvents();
    *durations = profiler->GetEventDurations();
  }
}

#endif

//********************************
// Create a person_detect model runner.
//********************************
void person_detect_model_runner_create(model_runner_t *ctx, void *buffer) {
  ctx->hInterpreter = buffer;  // NOTE: buffer can be NULL
  ctx->resolver_get_fun = &person_detect_resolver_get;
#ifndef NDEBUG
  ctx->profiler_get_fun = &person_detect_profiler_get;
  ctx->profiler_reset_fun = &person_detect_profiler_reset;
  ctx->profiler_durations_get_fun = &person_detect_profiler_durations_get;
#endif
}