// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
//#include "dispatch_queue_rtos.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/assert.h>

#include "dispatch_group.h"
#include "dispatch_queue.h"
#include "dispatch_task.h"
#include "dispatch_types.h"
#include "event_counter.h"
#include "rtos_osal.h"

#include "rtos_interrupt.h"

//***********************
//***********************
//***********************
// Worker
//***********************
//***********************
//***********************

void dispatch_queue_worker(void *param) {
  rtos_osal_queue_t *queue = (rtos_osal_queue_t *)param;
  dispatch_task_t *task = NULL;

  dispatch_queue_log("dispatch_queue_worker started\n");

  for (;;) {
    if (rtos_osal_queue_receive(queue, &task, RTOS_OSAL_WAIT_FOREVER) ==
        RTOS_OSAL_SUCCESS) {
      dispatch_queue_log("dispatch_queue_worker received task=%u\n",
                         (size_t)task);

      dispatch_task_perform(task);

      if (task->waitable) {
        // signal the event counter
        event_counter_signal(task->event_counter);
      } else {
        // the contract is that the worker must delete non-waitable tasks
        // FIXME
        // dispatch_task_delete(task);
      }
    }
  }
}

//***********************
//***********************
//***********************
// Queue struct
//***********************
//***********************
//***********************
struct dispatch_queue_struct {
  size_t thread_count;
  size_t thread_stack_size;
  rtos_osal_queue_t queue;
  rtos_osal_thread_t *threads;
};

//***********************
//***********************
//***********************
// Queue implementation
//***********************
//***********************
//***********************

dispatch_queue_t *dispatch_queue_create(size_t length, size_t thread_count,
                                        size_t thread_stack_size,
                                        size_t thread_priority) {
  dispatch_queue_log("dispatch_queue_create: length=%d, thread_count=%d\n",
                     length, thread_count);

  dispatch_queue_t *dispatch_queue = rtos_osal_malloc(sizeof(dispatch_queue_t));

  dispatch_queue->thread_count = thread_count;
  dispatch_queue->thread_stack_size = thread_stack_size;

  // allocate FreeRTOS queue
  rtos_osal_queue_create(&dispatch_queue->queue, "", length,
                         sizeof(dispatch_task_t *));

  // allocate threads
  dispatch_queue->threads =
      rtos_osal_malloc(sizeof(rtos_osal_thread_t) * thread_count);

  // initialize the queue
  dispatch_queue_init(dispatch_queue, thread_priority);

  rtos_printf("dispatch_queue_create: %u\n", (size_t)dispatch_queue);

  return dispatch_queue;
}

void dispatch_queue_init(dispatch_queue_t *dispatch_queue,
                         size_t thread_priority) {
  xassert(dispatch_queue);

  dispatch_queue_log("dispatch_queue_init: %u\n", (size_t)dispatch_queue);

  // create workers
  for (int i = 0; i < dispatch_queue->thread_count; i++) {
    rtos_osal_thread_create(&dispatch_queue->threads[i], "",
                            dispatch_queue_worker,
                            (void *)&dispatch_queue->queue,
                            dispatch_queue->thread_stack_size, thread_priority);
  }
}

void dispatch_queue_task_add(dispatch_queue_t *dispatch_queue,
                             dispatch_task_t *task) {
  xassert(dispatch_queue);
  xassert(task);

  dispatch_queue_log("dispatch_queue_add_task: %u   task=%u\n",
                     (size_t)dispatch_queue, (size_t)task);

  if (task->waitable) {
    task->event_counter = event_counter_create(1);
  }

  // send to queue
  rtos_osal_queue_send(&dispatch_queue->queue, (void *)&task,
                       RTOS_OSAL_WAIT_FOREVER);
}

void dispatch_queue_group_add(dispatch_queue_t *dispatch_queue,
                              dispatch_group_t *group) {
  xassert(dispatch_queue);
  xassert(group);

  dispatch_queue_log("dispatch_queue_group_add: %u   group=%u\n",
                     (size_t)dispatch_queue, (size_t)group);

  // event_counter_t *counter = NULL;

  // if (group->waitable) {
  //   // create event counter
  //   counter = event_counter_create(group->count);
  // }

  if (group->waitable) {
    // init event counter
    event_counter_init(group->event_counter, group->count);
  }

  // send to queue
  for (int i = 0; i < group->count; i++) {
    // group->tasks[i]->event_counter = counter;
    rtos_osal_queue_send(&dispatch_queue->queue, (void *)&group->tasks[i],
                         RTOS_OSAL_WAIT_FOREVER);
  }
}

void dispatch_queue_task_wait(dispatch_queue_t *dispatch_queue,
                              dispatch_task_t *task) {
  xassert(task);
  xassert(task->waitable);

  dispatch_queue_log("dispatch_queue_task_wait: %u   task=%u\n",
                     (size_t)dispatch_queue, (size_t)task);

  if (task->waitable) {
    xassert(task->event_counter);
    event_counter_wait(task->event_counter);
    // the contract is that the dispatch queue must delete waitable tasks
    // event_counter_delete(counter);
    // FIXME dispatch_task_delete(task);
  }
}

void dispatch_queue_group_wait(dispatch_queue_t *dispatch_queue,
                               dispatch_group_t *group) {
  xassert(group);
  xassert(group->waitable);

  dispatch_queue_log("dispatch_queue_group_wait: %u   group=%u\n",
                     (size_t)dispatch_queue, (size_t)group);

  if (group->waitable) {
    // can pick any task in the group to wait on because they
    // share the same event counter
    event_counter_t *counter = group->tasks[0]->event_counter;
    xassert(counter);
    event_counter_wait(counter);
    // event_counter_delete(counter);
    // FIXME
    // for (int i = 0; i < group->count; i++) {
    //   dispatch_task_delete(group->tasks[i]);
    // }
  }
}

void dispatch_queue_delete(dispatch_queue_t *dispatch_queue) {
  xassert(dispatch_queue);

  dispatch_queue_log("dispatch_queue_delete: %u\n", (size_t)dispatch_queue);

  // delete all threads
  for (int i = 0; i < dispatch_queue->thread_count; i++) {
    rtos_osal_thread_delete(&dispatch_queue->threads[i]);
  }

  // free memory
  rtos_osal_queue_delete(&dispatch_queue->queue);
  rtos_osal_free((void *)dispatch_queue->threads);
  rtos_osal_free((void *)dispatch_queue);
}
