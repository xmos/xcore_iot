// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_QUEUE_H_
#define DISPATCH_QUEUE_H_

#include <stddef.h>

#include "dispatch_group.h"
#include "dispatch_task.h"

typedef struct dispatch_queue_struct dispatch_queue_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Create a new dispatch queue
 *
 * \param length             Maximum number of tasks in the queue
 * \param thread_count       Number of thread workers
 * \param thread_stack_size  Size (in words) of the stack for each thread
 * worker
 * \param thread_priority    Priority for each thread worker.
 *
 * @return                   New dispatch queue object
 */
dispatch_queue_t* dispatch_queue_create(size_t length, size_t thread_count,
                                        size_t thread_stack_size,
                                        size_t thread_priority);

/** Initialize a new dispatch queue
 *
 * \param dispatch_queue   Dispatch queue object
 * \param thread_priority  Priority for each thread worker.
 */
void dispatch_queue_init(dispatch_queue_t* dispatch_queue,
                         size_t thread_priority);

/** Free memory allocated by dispatch_queue_create
 *
 * \param dispatch_queue  Dispatch queue object
 */
void dispatch_queue_delete(dispatch_queue_t* dispatch_queue);

/** Add a task to the dispatch queue.  If the dispatch queue is full,
 * this function will block in the callers thread until it can be added to the
 * queue.
 *
 * \param dispatch_queue  Dispatch queue object
 * \param task            Task object
 *
 */
void dispatch_queue_task_add(dispatch_queue_t* dispatch_queue,
                             dispatch_task_t* task);

/** Add a group to the dispatch queue.  If the dispatch queue is full,
 * this function will block in the callers thread until all the tasks in the
 * group can be added to the queue.
 *
 * \param dispatch_queue  Dispatch queue object
 * \param group           Group object
 *
 */
void dispatch_queue_group_add(dispatch_queue_t* dispatch_queue,
                              dispatch_group_t* group);

/** Creates a task and adds it to the the queue.  If the dispatch queue is full,
 * this function will block in the callers thread until it can be added to the
 * queue.
 *
 * \param dispatch_queue  Dispatch queue object
 * \param function        Function to perform, signature must be
 * <tt>void(void*)</tt> \param argument        Function argument \param waitable
 * The created task is waitable if TRUE, otherwise the task can not be waited on
 *
 * \return                Task object
 */
static inline dispatch_task_t* dispatch_queue_function_add(
    dispatch_queue_t* dispatch_queue, dispatch_function_t function,
    void* argument, bool waitable) {
  dispatch_task_t* task;

  task = dispatch_task_create(function, argument, waitable);
  dispatch_queue_task_add(dispatch_queue, task);

  return task;
}

/** Wait synchronously in the caller's thread for the task to finish executing
 *
 * \param dispatch_queue  Dispatch queue object
 * \param task            Task object, must be waitable
 */
void dispatch_queue_task_wait(dispatch_queue_t* dispatch_queue,
                              dispatch_task_t* task);

/** Wait synchronously in the caller's thread for the group to finish executing
 *
 * \param dispatch_queue  Dispatch queue object
 * \param group           Group object, must be waitable
 */
void dispatch_queue_group_wait(dispatch_queue_t* dispatch_queue,
                               dispatch_group_t* group);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // DISPATCH_QUEUE_H_