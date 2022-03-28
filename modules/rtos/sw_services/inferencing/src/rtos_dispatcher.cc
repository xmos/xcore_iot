// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_dispatcher.h"

namespace tflite {
namespace micro {
namespace xcore {

RTOSDispatcher::RTOSDispatcher(dispatcher_t *dispatcher)
    : dispatcher_(dispatcher) {
  group_ = dispatch_group_create(kMaxThreads);
  for (size_t i = 0; i < kMaxThreads; i++) {
    dispatch_group_job_add(group_, dispatch_job_create(nullptr, nullptr));
  }
}

RTOSDispatcher::~RTOSDispatcher() {
  dispatch_job_t **jobs = dispatch_group_jobs_get(group_);
  for (size_t i = 0; i < kMaxThreads; i++) {
    dispatch_job_delete(jobs[i]);
  }
  dispatch_group_delete(group_);
}

TfLiteStatus RTOSDispatcher::Invoke(void **arguments, size_t size) const {
  size_t num_threads_added = 0;
  dispatch_job_t **jobs = dispatch_group_jobs_get(group_);

  // reset the group for first iteration
  dispatch_group_init(group_);
  // printf("Invoke  size=%d      %u\n", size, (size_t)function_);

  for (int i = 0; i < size; i++) {
    dispatch_job_init(jobs[i], function_, arguments[i]);
    dispatch_group_job_add(group_, jobs[i]);

    num_threads_added++;
    if (num_threads_added == num_threads_) {
      dispatcher_group_add(dispatcher_, group_);
      dispatcher_group_wait(dispatcher_, group_);
      // reset the group for next iteration
      dispatch_group_init(group_);
      num_threads_added = 0;
    }
  }

  if (num_threads_added > 0) {
    dispatcher_group_add(dispatcher_, group_);
    dispatcher_group_wait(dispatcher_, group_);
  }

  return kTfLiteOk;
}

} // namespace xcore
} // namespace micro
} // namespace tflite