// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "dispatch_group.h"

#include <stdlib.h>
#include <string.h>

#include "rtos_osal.h"
#include "dispatch_types.h"

dispatch_group_t *dispatch_group_create(size_t length, bool waitable) {
  xassert(length > 0);
  dispatch_group_t *group;

  dispatch_queue_log("dispatch_group_create: length=%d\n", length);

  group = rtos_osal_malloc(sizeof(dispatch_group_t));

  group->length = length;
  group->tasks = rtos_osal_malloc(sizeof(dispatch_task_t *) * length);

  if (waitable) {
    // create event counter
    group->event_counter = event_counter_create(group->count);
  } else {
    group->event_counter = NULL;
  }

  // initialize the queue
  dispatch_group_init(group, waitable);

  return group;
}

void dispatch_group_init(dispatch_group_t *group, bool waitable) {
  xassert(group);

  group->waitable = waitable;
  group->count = 0;
  if (group->waitable) {
    xassert(group->event_counter);
    event_counter_init(group->event_counter, group->length);
  }
}

dispatch_task_t **dispatch_group_tasks_get(dispatch_group_t *group) {
  xassert(group);
  return group->tasks;
}

dispatch_task_t *dispatch_group_function_add(dispatch_group_t *group,
                                             dispatch_function_t function,
                                             void *argument) {
  dispatch_task_t *task;

  task = dispatch_task_create(function, argument, group->waitable);
  dispatch_group_task_add(group, task);

  return task;
}

void dispatch_group_task_add(dispatch_group_t *group, dispatch_task_t *task) {
  xassert(group);
  xassert(group->count < group->length);
  xassert(task);

  task->waitable = group->waitable;
  task->event_counter = group->event_counter;
  group->tasks[group->count] = task;
  group->count++;
}

void dispatch_group_perform(dispatch_group_t *group) {
  xassert(group);

  dispatch_queue_log("dispatch_group_perform: %u\n", (size_t)group);

  // call group in current thread
  for (int i = 0; i < group->count; i++) {
    dispatch_task_t *task = group->tasks[i];
    dispatch_task_perform(task);
  }
}

void dispatch_group_delete(dispatch_group_t *group) {
  xassert(group);
  xassert(group->tasks);

  dispatch_queue_log("dispatch_group_delete: %u\n", (size_t)group);

  if (group->waitable) {
    xassert(group->event_counter);
    event_counter_delete(group->event_counter);
  }
  rtos_osal_free(group->tasks);
  rtos_osal_free(group);
}
