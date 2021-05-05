// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_EVENT_COUNTER_H_
#define DISPATCH_EVENT_COUNTER_H_

#include <stddef.h>

typedef struct event_counter_struct event_counter_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

event_counter_t *event_counter_create(size_t count);
void event_counter_signal(event_counter_t *counter);
void event_counter_wait(event_counter_t *counter);
void event_counter_delete(event_counter_t *counter);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // DISPATCH_EVENT_COUNTER_H_