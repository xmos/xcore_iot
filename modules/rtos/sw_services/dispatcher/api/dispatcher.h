// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "dispatch_group.h"
#include "dispatch_job.h"

#define DISPATCHER_JOB_ATTRIBUTE __attribute__((fptrgroup("dispatcher_job")))

typedef struct dispatcher_struct dispatcher_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Create a new dispatcher
 *
 * @return  New dispatcher object
 */
dispatcher_t *dispatcher_create();

/** Free memory allocated by dispatcher_create
 *
 * \param dispatcher  Dispatcher object
 */
void dispatcher_delete(dispatcher_t *dispatcher);

/** Initialize a dispatcher with thread workers
 *
 * \param dispatcher       Dispatcher object
 * \param length           Maximum number of tasks in the queue
 * \param thread_count     Number of thread workers
 * \param thread_priority  Priority for each thread worker.
 */
void dispatcher_thread_init(dispatcher_t *dispatcher, size_t length,
                            size_t thread_count, size_t thread_priority);

/** Initialize a dispatcher with ISR workers
 *
 * \param dispatcher  Dispatcher object
 * \param core_map    Cores to use for ISRs
 */
void dispatcher_isr_init(dispatcher_t *dispatcher, uint32_t corecore_map_mask);

/** Add a job to the dispatcher.
 *
 * If the dispatcher is initialized with threads and the queue is full, this
 * function will block in the callers thread until the can be dispatched.
 *
 * \param dispatcher  Dispatcher object
 * \param job         Job object
 *
 */
void dispatcher_job_add(dispatcher_t *dispatcher, dispatch_job_t *job);

/** Add a group to the dispatcher.
 *
 * If the dispatcher is initialized with threads and the queue is full, this
 * function will block in the callers thread until all the jobs in the group
 * can be dispatched.
 *
 * \param dispatcher  Dispatcher object
 * \param group       Group object
 *
 */
void dispatcher_group_add(dispatcher_t *dispatcher, dispatch_group_t *group);

/** Creates a job and adds it to the dispatcher.
 *
 * If the dispatcher is initialized with threads and the queue is full, this
 * function will block in the callers thread until the function can be
 * dispatched.
 *
 * \param dispatcher      Dispatcher object
 * \param function        Function to perform, signature must be
 * <tt>void(void*)</tt>
 * \param argument        Function argument
 *
 * \return                Job object
 */
static inline dispatch_job_t *
dispatcher_function_add(dispatcher_t *dispatcher, dispatch_function_t function,
                        void *argument) {
  dispatch_job_t *job;

  job = dispatch_job_create(function, argument);
  dispatcher_job_add(dispatcher, job);

  return job;
}

/** Wait synchronously in the caller's thread for the job to finish executing
 *
 * \param dispatcher  Dispatcher object
 * \param job         Job object
 */
void dispatcher_job_wait(dispatcher_t *dispatcher, dispatch_job_t *job);

/** Wait synchronously in the caller's thread for the group to finish executing
 *
 * \param dispatcher  Dispatcher object
 * \param group       Group object
 */
void dispatcher_group_wait(dispatcher_t *dispatcher, dispatch_group_t *group);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DISPATCH_QUEUE_H_