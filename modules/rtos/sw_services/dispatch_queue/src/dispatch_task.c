// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "dispatch_task.h"

#include <xcore/assert.h>

#include "rtos_osal.h"
#include "dispatch_types.h"

dispatch_task_t *dispatch_task_create(dispatch_function_t function,
                                      void *argument, bool waitable) {
  dispatch_task_t *task;
  task = rtos_osal_malloc(sizeof(dispatch_task_t));

  dispatch_task_init(task, function, argument, waitable);

  dispatch_queue_log("dispatch_task_create:  task=%u\n", (size_t)task);

  return task;
}

void dispatch_task_init(dispatch_task_t *task, dispatch_function_t function,
                        void *argument, bool waitable) {
  xassert(task);

  task->function = function;
  task->argument = argument;
  task->waitable = waitable;
  task->event_counter = NULL;
}

void dispatch_task_perform(dispatch_task_t *task) {
  xassert(task);
  xassert(task->function);

  dispatch_queue_log("dispatch_task_perform:  task=%u\n", (size_t)task);

  // call function in current thread
  task->function(task->argument);
}

void dispatch_task_delete(dispatch_task_t *task) {
  xassert(task);

  dispatch_queue_log("dispatch_task_delete:  task=%u\n", (size_t)task);

  rtos_osal_free(task);
}
