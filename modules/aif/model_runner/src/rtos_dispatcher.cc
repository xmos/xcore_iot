// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_dispatcher.h"

namespace tflite {
namespace micro {
namespace xcore {

RTOSDispatcher::RTOSDispatcher(dispatch_queue_t* dispatch_queue)
    : dispatch_queue_(dispatch_queue) {
  group_ = dispatch_group_create(kMaxThreads, true);
  for (size_t i = 0; i < kMaxThreads; i++) {
    dispatch_group_task_add(group_,
                            dispatch_task_create(nullptr, nullptr, true));
  }
}

RTOSDispatcher::~RTOSDispatcher() {
  dispatch_task_t** tasks = dispatch_group_tasks_get(group_);
  for (size_t i = 0; i < kMaxThreads; i++) {
    dispatch_task_delete(tasks[i]);
  }
  dispatch_group_delete(group_);
}

TfLiteStatus RTOSDispatcher::Invoke(void** arguments, size_t size) const {
  size_t num_threads_added = 0;
  dispatch_task_t** tasks = dispatch_group_tasks_get(group_);

  dispatch_group_init(group_, true);

  for (int i = 0; i < size; i++) {
    // dispatch_group_function_add(group_, function_, arguments[i]);
    dispatch_task_init(tasks[i], function_, arguments[i], true);
    dispatch_group_task_add(group_, tasks[i]);

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