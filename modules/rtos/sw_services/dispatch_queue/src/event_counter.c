// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "event_counter.h"

#include <xcore/assert.h>

#include "FreeRTOS.h"
#include "rtos_osal.h"

struct event_counter_struct {
  rtos_osal_semaphore_t semaphore;
  size_t count;
};

event_counter_t *event_counter_create(size_t count) {
  event_counter_t *counter = rtos_osal_malloc(sizeof(event_counter_t));

  rtos_osal_semaphore_create(&counter->semaphore, "", 1, 0);

  event_counter_init(counter, count);
  return counter;
}

void event_counter_init(event_counter_t *counter, size_t count) {
  xassert(counter);

  counter->count = count;
}

void event_counter_signal(event_counter_t *counter) {
  xassert(counter);

  int signal = 0;

  int state = rtos_osal_critical_enter();
  if (counter->count > 0) {
    if (--counter->count == 0) {
      signal = 1;
    }
  }
  rtos_osal_critical_exit(state);

  if (signal) {
    rtos_osal_semaphore_put(&counter->semaphore);
  }
}

void event_counter_wait(event_counter_t *counter) {
  xassert(counter);

  rtos_osal_semaphore_get(&counter->semaphore, RTOS_OSAL_WAIT_FOREVER);
}

void event_counter_delete(event_counter_t *counter) {
  xassert(counter);

  rtos_osal_semaphore_delete(&counter->semaphore);
  rtos_osal_free(counter);
}
