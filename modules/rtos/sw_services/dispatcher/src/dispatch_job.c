// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "dispatch_job.h"

#include <xcore/assert.h>

#include "rtos_osal.h"
#include "dispatch_types.h"

dispatch_job_t *dispatch_job_create(dispatch_function_t function,
                                    void *argument) {
  dispatch_job_t *task;
  task = rtos_osal_malloc(sizeof(dispatch_job_t));

  dispatch_job_init(task, function, argument);

  dispatcher_log("dispatch_job_create:  task=%u\n", (size_t)task);

  return task;
}

void dispatch_job_init(dispatch_job_t *task, dispatch_function_t function,
                       void *argument) {
  xassert(task);

  task->function = function;
  task->argument = argument;
  task->event_counter = NULL;
}

void dispatch_job_perform(dispatch_job_t *task) {
  xassert(task);
  xassert(task->function);

  dispatcher_log("dispatch_job_perform:  task=%u\n", (size_t)task);

  // call function in current thread
  task->function(task->argument);
}

void dispatch_job_delete(dispatch_job_t *task) {
  xassert(task);

  dispatcher_log("dispatch_job_delete:  task=%u\n", (size_t)task);

  rtos_osal_free(task);
}
