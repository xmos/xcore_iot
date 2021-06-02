// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_dispatcher.h"

namespace tflite {
namespace micro {
namespace xcore {

RTOSDispatcher::RTOSDispatcher(dispatch_queue_t* dispatch_queue)
    : dispatch_queue_(dispatch_queue) {
  group_ = dispatch_group_create(kMaxThreads, true);
}

RTOSDispatcher::~RTOSDispatcher() { dispatch_group_delete(group_); }

TfLiteStatus RTOSDispatcher::Invoke(void** arguments, size_t size) const {
  size_t num_threads_added = 0;

  for (int i = 0; i < size; i++) {
    dispatch_group_function_add(group_, function_, arguments[i]);

    num_threads_added++;
    if (num_threads_added == num_threads_) {
      dispatch_queue_group_add(dispatch_queue_, group_);
      dispatch_queue_group_wait(dispatch_queue_, group_);
      dispatch_group_init(group_, true);
      num_threads_added = 0;
    }
  }

  if (num_threads_added > 0) {
    dispatch_queue_group_add(dispatch_queue_, group_);
    dispatch_queue_group_wait(dispatch_queue_, group_);
  }

  return kTfLiteOk;
}

}  // namespace xcore
}  // namespace micro
}  // namespace tflite