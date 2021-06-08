// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_JOB_H_
#define DISPATCH_JOB_H_

#include <stdbool.h>
#include <stddef.h>

typedef void (*dispatch_function_t)(void *);

typedef struct dispatch_job_struct dispatch_job_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Create a new task, non-waitable task
 *
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 *
 * \return              Task object
 */
dispatch_job_t *dispatch_job_create(dispatch_function_t function,
                                    void *argument);

/** Initialize a task
 *
 * \param task      Task object
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 */
void dispatch_job_init(dispatch_job_t *task, dispatch_function_t function,
                       void *argument);

/** Run the task in the caller's thread
 *
 * \param task  Task object
 */
void dispatch_job_perform(dispatch_job_t *task);

/** Destroy the task
 *
 * \param task  Task object
 */
void dispatch_job_delete(dispatch_job_t *task);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DISPATCH_JOB_H_