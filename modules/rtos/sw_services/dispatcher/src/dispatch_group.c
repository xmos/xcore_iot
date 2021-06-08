// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "dispatch_group.h"

#include <stdlib.h>
#include <string.h>

#include "rtos_osal.h"
#include "dispatch_types.h"

dispatch_group_t *dispatch_group_create(size_t length) {
  xassert(length > 0);
  dispatch_group_t *group;

  dispatcher_log("dispatch_group_create: length=%d\n", length);

  group = rtos_osal_malloc(sizeof(dispatch_group_t));

  group->length = length;
  group->jobs = rtos_osal_malloc(sizeof(dispatch_job_t *) * length);

  // group->event_counter = event_counter_create(group->count);
  group->event_counter = NULL;

  // initialize the queue
  dispatch_group_init(group);

  return group;
}

void dispatch_group_init(dispatch_group_t *group) {
  xassert(group);

  group->count = 0;

  if (group->event_counter)
    event_counter_init(group->event_counter, group->length);
}

dispatch_job_t **dispatch_group_jobs_get(dispatch_group_t *group) {
  xassert(group);
  return group->jobs;
}

dispatch_job_t *dispatch_group_function_add(dispatch_group_t *group,
                                            dispatch_function_t function,
                                            void *argument) {
  dispatch_job_t *job;

  job = dispatch_job_create(function, argument);
  dispatch_group_job_add(group, job);

  return job;
}

void dispatch_group_job_add(dispatch_group_t *group, dispatch_job_t *job) {
  xassert(group);
  xassert(group->count < group->length);
  xassert(job);

  group->jobs[group->count] = job;
  group->count++;
}

void dispatch_group_perform(dispatch_group_t *group) {
  xassert(group);

  dispatcher_log("dispatch_group_perform: %u\n", (size_t)group);

  // call group in current thread
  for (int i = 0; i < group->count; i++) {
    dispatch_job_t *job = group->jobs[i];
    dispatch_job_perform(job);
  }
}

void dispatch_group_delete(dispatch_group_t *group) {
  xassert(group);
  xassert(group->jobs);

  dispatcher_log("dispatch_group_delete: %u\n", (size_t)group);

  if (group->event_counter)
    event_counter_delete(group->event_counter);
  rtos_osal_free(group->jobs);
  rtos_osal_free(group);
}
