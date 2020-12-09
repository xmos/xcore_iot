// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef MODEL_RUNNER_PROFILER_H_
#define MODEL_RUNNER_PROFILER_H_

#include <ctime>

extern "C" {
#ifdef _TIME_H_
#define _clock_defined
#endif
}

#include <platform.h>  // for PLATFORM_REFERENCE_MHZ

#include "tensorflow/lite/core/api/profiler.h"
#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/micro_time.h"

namespace xcore {

class ModelRunnerProfiler : public tflite::Profiler {
 public:
  explicit ModelRunnerProfiler(uint32_t* event_times, size_t max_count)
      : event_count_(0),
        event_times_(event_times),
        max_event_count_(max_count) {}
  ~ModelRunnerProfiler() override = default;

  // AddEvent is unused for TFLu.
  void AddEvent(const char* tag, EventType event_type, uint64_t start,
                uint64_t end, int64_t event_metadata1,
                int64_t event_metadata2) override{};

  // BeginEvent followed by code followed by EndEvent will profile the code
  // enclosed. Multiple concurrent events are unsupported, so the return value
  // is always 0. Event_metadata1 and event_metadata2 are unused.
  uint32_t BeginEvent(const char* tag, EventType event_type,
                      int64_t event_metadata1, int64_t event_metadata2) {
    event_start_time_ = tflite::GetCurrentTimeTicks();
    return 0;
  }

  void EndEvent(uint32_t event_handle) {
    uint32_t event_duration;
    int32_t event_end_time = tflite::GetCurrentTimeTicks();
    event_duration =
        (event_end_time - event_start_time_) / PLATFORM_REFERENCE_MHZ;

    if (event_count_ < max_event_count_) {
      event_times_[event_count_] = event_duration;
      event_count_++;
    }
  }

  void Reset() { event_count_ = 0; }

  uint32_t const* GetTimes() { return event_times_; }

  uint32_t GetNumTimes() { return event_count_; }

 private:
  uint32_t event_start_time_;
  uint32_t event_count_;
  uint32_t* event_times_;
  uint32_t max_event_count_;
  TF_LITE_REMOVE_VIRTUAL_DELETE
};

}  // namespace xcore

#endif  // MODEL_RUNNER_PROFILER_H_
