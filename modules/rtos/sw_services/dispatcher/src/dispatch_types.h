// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_TYPES_H_
#define DISPATCH_TYPES_H_

#include <stdbool.h>
#include <stddef.h>

#include "dispatcher.h"
#include "event_counter.h"

// the following line can be used when debugging actions on the dispatch queue
#define dispatcher_log(...) // rtos_printf(__VA_ARGS__)

struct dispatch_job_struct {
  DISPATCHER_JOB_ATTRIBUTE dispatch_function_t
      function;                   // the function to perform
  void *argument;                 // argument to pass to the function
  event_counter_t *event_counter; // event counter used to wait
};

struct dispatch_group_struct {
  size_t length;                  // maximum number of jobs in the group
  size_t count;                   // number of jobs added to the group
  dispatch_job_t **jobs;          // array of job pointers
  event_counter_t *event_counter; // event counter used to wait
};

#endif // DISPATCH_TYPES_H_