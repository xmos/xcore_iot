// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "event_counter.h"

#include <xcore/assert.h>

#include "FreeRTOS.h"
#include "rtos_osal.h"

struct event_counter_struct {
  rtos_osal_semaphore_t *semaphore;
  volatile size_t count;
};

event_counter_t *event_counter_create(size_t count, WorkerType worker_type) {
  event_counter_t *counter = rtos_osal_malloc(sizeof(event_counter_t));

  counter->semaphore = NULL;

  if (worker_type == ThreadWorker) {
    counter->semaphore = rtos_osal_malloc(sizeof(rtos_osal_semaphore_t));
    rtos_osal_semaphore_create(counter->semaphore, "", 1, 0);
  }

  event_counter_init(counter, count);
  return counter;
}

void event_counter_init(event_counter_t *counter, size_t count) {
  xassert(counter);

  counter->count = count;
}

int event_counter_signal(event_counter_t *counter, WorkerType worker_type) {
  xassert(counter);

  int state;
  int signal = 0;

  // guard the count
  if (worker_type == ThreadWorker) {
    state = rtos_osal_critical_enter();
  } else if (worker_type == ISRWorker) {
    rtos_lock_acquire(0);
  }

  // update the count
  if (counter->count > 0) {
    if (--counter->count == 0) {
      signal = 1;
    }
  }

  // unguard the count
  if (worker_type == ThreadWorker) {
    rtos_osal_critical_exit(state);
    if (signal) {
      rtos_osal_semaphore_put(counter->semaphore);
    }
  } else if (worker_type == ISRWorker) {
    rtos_lock_release(0);
  }

  return signal;
}

void event_counter_wait(event_counter_t *counter, WorkerType worker_type) {
  xassert(counter);

  if (worker_type == ThreadWorker) {
    rtos_osal_semaphore_get(counter->semaphore, RTOS_OSAL_WAIT_FOREVER);
  } else if (worker_type == ISRWorker) {
    while (counter->count > 0)
      ;
  }
}

void event_counter_delete(event_counter_t *counter) {
  xassert(counter);

  if (counter->semaphore) {
    rtos_osal_semaphore_delete(counter->semaphore);
    rtos_osal_free(counter->semaphore);
  }

  rtos_osal_free(counter);
}
