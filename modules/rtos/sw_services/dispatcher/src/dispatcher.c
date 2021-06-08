// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/channel.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

#include "dispatcher.h"
#include "dispatch_types.h"
#include "event_counter.h"
#include "rtos_interrupt.h"
#include "rtos_osal.h"
#include "worker_types.h"

//***********************
//***********************
//***********************
// ISR Worker
//***********************
//***********************
//***********************

#define GROUP_CHANNEL_WAIT 1
#define MAX_CORE_COUNT (8)

static inline void chanend_job_send(chanend_t src, chanend_t dst,
                                    dispatch_job_t *job) {
  chanend_set_dest(src, dst);
  s_chan_out_word(src, (uint32_t)job);
  chanend_out_control_token(src, XS1_CT_PAUSE);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(dispatcher_isr_worker, arg) {
  dispatch_job_t *job = NULL;
  chanend_t *chanend = arg;

  job = (dispatch_job_t *)s_chan_in_word(*chanend);

  dispatcher_log("dispatcher_isr_worker received job=%u\n", (size_t)job);

  // dispatcher_log("Minimum heap free: %d\n\tCurrent heap free: %d\n",
  //                xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());

  if (job) {
    dispatch_job_perform(job);

    // signal the event counter
    if (event_counter_signal(job->event_counter, ISRWorker)) {
#if GROUP_CHANNEL_WAIT
      chanend_out_end_token(*chanend);
#endif
    }
  } else {
    // a NULL job means it is time to free my chanend
    triggerable_disable_trigger(*chanend);
    chanend_free(*chanend);
  }
}

//***********************
//***********************
//***********************
// Thread Worker
//***********************
//***********************
//***********************

void dispatcher_thread_worker(void *param) {
  rtos_osal_queue_t *queue = (rtos_osal_queue_t *)param;
  dispatch_job_t *job = NULL;

  dispatcher_log("dispatcher_thread_worker started\n");

  for (;;) {
    if (rtos_osal_queue_receive(queue, &job, RTOS_OSAL_WAIT_FOREVER) ==
        RTOS_OSAL_SUCCESS) {
      dispatcher_log("dispatcher_thread_worker received job=%u  at=%u\n",
                     (size_t)job,
                     get_reference_time() / PLATFORM_REFERENCE_MHZ);

      dispatch_job_perform(job);

      // signal the event counter
      event_counter_signal(job->event_counter, ThreadWorker);
    }
  }
}

//***********************
//***********************
//***********************
// Dispatcher
//***********************
//***********************
//***********************

struct dispatcher_struct {
  WorkerType worker_type;
  size_t worker_count;
  // thread worker state
  rtos_osal_queue_t queue;
  rtos_osal_thread_t *threads;
  // isr worker state
  chanend_t chanend;
  chanend_t *isr_chanends;
};

dispatcher_t *dispatcher_create() {
  dispatcher_t *dispatcher = rtos_osal_malloc(sizeof(dispatcher_t));

  dispatcher->worker_type = UninitializedWorker;

  dispatcher->worker_count = 0;
  dispatcher->threads = NULL;

  dispatcher->isr_chanends = NULL;

  dispatcher_log("dispatcher_create: %u\n", (size_t)dispatcher);

  return dispatcher;
}

void dispatcher_delete(dispatcher_t *dispatcher) {
  dispatcher_log("dispatcher_delete: %u\n", (size_t)dispatcher);
  xassert(dispatcher);

  if (dispatcher->worker_type == ThreadWorker) {
    rtos_osal_queue_delete(&dispatcher->queue);
    for (int i = 0; i < dispatcher->worker_count; i++) {
      rtos_osal_thread_delete(&dispatcher->threads[i]);
    }
    rtos_osal_free((void *)dispatcher->threads);
  } else if (dispatcher->worker_type == ISRWorker) {
    // send all ISR workers a NULL job which instructs them to free their
    // chanend
    // disable interrupts while dispatching jobs
    uint32_t mask = rtos_interrupt_mask_all();
    for (int i = 0; i < dispatcher->worker_count; i++) {
      chanend_job_send(dispatcher->chanend, dispatcher->isr_chanends[i], NULL);
    }
    // re-enable interupts
    rtos_interrupt_mask_set(mask);
    chanend_free(dispatcher->chanend);
    rtos_osal_free((void *)dispatcher->isr_chanends);
  }

  rtos_osal_free((void *)dispatcher);
}

void dispatcher_thread_init(dispatcher_t *dispatcher, size_t length,
                            size_t thread_count, size_t thread_priority) {
  dispatcher_log("dispatcher_thread_init: %u   length=%d, thread_count=%d, , "
                 "thread_priority=%d\n",
                 dispatcher, length, thread_count, thread_priority);
  xassert(dispatcher);
  xassert(length > 0);
  xassert(thread_count > 0);
  xassert(dispatcher->worker_type == UninitializedWorker);

  dispatcher->worker_type = ThreadWorker;
  dispatcher->worker_count = thread_count;

  // allocate FreeRTOS queue
  rtos_osal_queue_create(&dispatcher->queue, "", length,
                         sizeof(dispatch_job_t *));

  // allocate threads
  dispatcher->threads =
      rtos_osal_malloc(sizeof(rtos_osal_thread_t) * dispatcher->worker_count);

  // create workers
  for (int i = 0; i < dispatcher->worker_count; i++) {
    rtos_osal_thread_create(
        &dispatcher->threads[i], "", dispatcher_thread_worker,
        (void *)&dispatcher->queue,
        RTOS_THREAD_STACK_SIZE(dispatcher_thread_worker), thread_priority);
  }
}

void dispatcher_isr_init(dispatcher_t *dispatcher, uint32_t core_map) {
  dispatcher_log("dispatcher_isr_init: %u   core_map=%d\n", dispatcher,
                 core_map);
  xassert(dispatcher);
  xassert(dispatcher->worker_type == UninitializedWorker);

  dispatcher->worker_type = ISRWorker;

  uint32_t core_exclude_map;
  rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);

  // create the dispatcher's chanend
  dispatcher->chanend = chanend_alloc();
  xassert(dispatcher->chanend);

  // count bits in the core mask to determine worker count
  uint32_t pending = core_map;
  uint32_t core_id;
  while (pending != 0) {
    core_id = 31UL - (uint32_t)__builtin_clz(pending);
    xassert(core_id <= MAX_CORE_COUNT);
    pending &= ~(1 << core_id);

    dispatcher->worker_count++;
  }
  xassert(dispatcher->worker_count > 0);

  // create the ISR's chanends
  dispatcher->isr_chanends =
      rtos_osal_malloc(sizeof(chanend_t) * dispatcher->worker_count);
  xassert(dispatcher->isr_chanends);

  // setup ISRs on core set bits in the core_map
  int index = 0;
  pending = core_map;

  while (pending != 0) {
    core_id = 31UL - (uint32_t)__builtin_clz(pending);
    pending &= ~(1 << core_id);

    // create ISR's chanend
    dispatcher->isr_chanends[index] = chanend_alloc();
    xassert(dispatcher->isr_chanends[index]);
    chanend_set_dest(dispatcher->isr_chanends[index], dispatcher->chanend);

    // set ISR on specified core id
    //   NOTE: rtos_osal_thread_core_exclusion_set switches this code's
    //   execution to the specified core id
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << core_id));
    triggerable_setup_interrupt_callback(
        dispatcher->isr_chanends[index], &dispatcher->isr_chanends[index],
        RTOS_INTERRUPT_CALLBACK(dispatcher_isr_worker));
    triggerable_enable_trigger(dispatcher->isr_chanends[index]);

    index++;
  }

  // Restore the core exclusion map for the calling thread
  rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);
}

void dispatcher_job_add(dispatcher_t *dispatcher, dispatch_job_t *job) {
  dispatcher_log("dispatcher_add_job: %u   job=%u  worker_type=%d\n",
                 (size_t)dispatcher, (size_t)job, dispatcher->worker_type);
  xassert(dispatcher);
  xassert(job);
  xassert(dispatcher->worker_type != UninitializedWorker);

  job->event_counter = event_counter_create(1, dispatcher->worker_type);

  if (dispatcher->worker_type == ThreadWorker) {
    // send to queue
    rtos_osal_queue_send(&dispatcher->queue, (void *)&job,
                         RTOS_OSAL_WAIT_FOREVER);
  } else if (dispatcher->worker_type == ISRWorker) {
    // disable interrupts while dispatching jobs
    uint32_t mask = rtos_interrupt_mask_all();
    chanend_job_send(dispatcher->chanend, dispatcher->isr_chanends[0], job);
    // re-enable interrupts
    rtos_interrupt_mask_set(mask);
  }
}

void dispatcher_group_add(dispatcher_t *dispatcher, dispatch_group_t *group) {
  dispatcher_log("dispatcher_group_add: %u   group=%u  worker_type=%d\n",
                 (size_t)dispatcher, (size_t)group, dispatcher->worker_type);
  xassert(dispatcher);
  xassert(group);
  xassert(dispatcher->worker_type != UninitializedWorker);

  // init event counter
  if (group->event_counter == NULL)
    group->event_counter =
        event_counter_create(group->count, dispatcher->worker_type);
  event_counter_init(group->event_counter, group->count);

  // dispatchjobs in group
  if (dispatcher->worker_type == ThreadWorker) {
    for (int i = 0; i < group->count; i++) {
      group->jobs[i]->event_counter = group->event_counter;

      rtos_osal_queue_send(&dispatcher->queue, (void *)&group->jobs[i],
                           RTOS_OSAL_WAIT_FOREVER);
    }
  } else if (dispatcher->worker_type == ISRWorker) {
    // disable interrupts while dispatching jobs
    uint32_t mask = rtos_interrupt_mask_all();
    for (int i = 0; i < group->count; i++) {
      group->jobs[i]->event_counter = group->event_counter;
      chanend_job_send(dispatcher->chanend, dispatcher->isr_chanends[i],
                       group->jobs[i]);
    }
    // re-enable interrupts
    rtos_interrupt_mask_set(mask);
  }
}

void dispatcher_job_wait(dispatcher_t *dispatcher, dispatch_job_t *job) {
  dispatcher_log("dispatcher_job_wait: %u   job=%u\n", (size_t)dispatcher,
                 (size_t)job);
  xassert(dispatcher);
  xassert(job);
  xassert(job->event_counter);

  if (dispatcher->worker_type == ThreadWorker) {
    event_counter_wait(job->event_counter, dispatcher->worker_type);
  } else if (dispatcher->worker_type == ISRWorker) {
#if GROUP_CHANNEL_WAIT
    chanend_check_end_token(dispatcher->chanend);
#else
    event_counter_wait(job->event_counter, dispatcher->worker_type);
#endif
  }
}

void dispatcher_group_wait(dispatcher_t *dispatcher, dispatch_group_t *group) {
  dispatcher_log("dispatcher_group_wait: %u   group=%u\n", (size_t)dispatcher,
                 (size_t)group);
  xassert(dispatcher);
  xassert(group);
  xassert(group->event_counter);

  // can pick any job in the group to wait on because they
  // share the same event counter
  event_counter_t *counter = group->jobs[0]->event_counter;
  xassert(counter);
  if (dispatcher->worker_type == ThreadWorker) {
    event_counter_wait(counter, dispatcher->worker_type);
  } else if (dispatcher->worker_type == ISRWorker) {
#if GROUP_CHANNEL_WAIT
    chanend_check_end_token(dispatcher->chanend);
#else
    event_counter_wait(counter, dispatcher->worker_type);
#endif
  }
}
