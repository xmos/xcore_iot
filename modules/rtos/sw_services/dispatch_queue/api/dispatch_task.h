// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_TASK_H_
#define DISPATCH_TASK_H_

#include <stdbool.h>
#include <stddef.h>

#define DISPATCH_TASK_FUNCTION \
  __attribute__((fptrgroup("dispatch_task_function")))

typedef void (*dispatch_function_t)(void *);

typedef struct dispatch_task_struct dispatch_task_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Create a new task, non-waitable task
 *
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 * \param waitable  The task is waitable if TRUE, otherwise the task can not
 * be waited on
 *
 * \return              Task object
 */
dispatch_task_t *dispatch_task_create(dispatch_function_t function,
                                      void *argument, bool waitable);

/** Initialize a task
 *
 * \param task      Task object
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 * \param waitable  The task is waitable if TRUE, otherwise the task can not
 * be waited on
 */
void dispatch_task_init(dispatch_task_t *task, dispatch_function_t function,
                        void *argument, bool waitable);

/** Run the task in the caller's thread
 *
 * \param task  Task object
 */
void dispatch_task_perform(dispatch_task_t *task);

/** Destroy the task
 *
 * \param task  Task object
 */
void dispatch_task_delete(dispatch_task_t *task);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // DISPATCH_TASK_H_