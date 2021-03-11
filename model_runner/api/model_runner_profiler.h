// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MODEL_RUNNER_PROFILER_H_
#define MODEL_RUNNER_PROFILER_H_

#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/micro_time.h"

namespace xcore {

template <unsigned int tMaxEventCount>
class ModelRunnerProfiler : public tflite::MicroProfiler {
 public:
  explicit ModelRunnerProfiler() : event_count_(0) {}
  ~ModelRunnerProfiler() override = default;

  uint32_t BeginEvent(const char* tag) {
    event_start_time_ = tflite::GetCurrentTimeTicks();
    return 0;
  }

  void EndEvent(uint32_t event_handle) {
    uint32_t event_duration;
    int32_t event_end_time = tflite::GetCurrentTimeTicks();
    event_duration = (event_end_time - event_start_time_);

    if (event_count_ < tMaxEventCount) {
      event_durations_[event_count_] = event_duration;
      event_count_++;
    }
  }

  uint32_t const* GetEventDurations() {return event_durations_;}
  uint32_t GetNumEvents() {return event_count_;}

 private:
  uint32_t event_start_time_;
  uint32_t event_count_;
  uint32_t event_durations_[tMaxEventCount];
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}  // namespace xcore

#endif  // MODEL_RUNNER_PROFILER_H_
