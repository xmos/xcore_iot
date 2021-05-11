// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1
#ifndef DISPATCH_GROUP_H_
#define DISPATCH_GROUP_H_

#include <stdbool.h>
#include <stddef.h>

#include "dispatch_task.h"

typedef struct dispatch_group_struct dispatch_group_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Create a new task group
 *
 * \param length    Maximum number of tasks in the group
 * \param waitable  The task is waitable if TRUE, otherwise the task can not
 *
 * \return          Group object
 */
dispatch_group_t *dispatch_group_create(size_t length, bool waitable);

/** Initialize a new task group
 *
 * \param group     Group object
 * \param waitable  The group is waitable if TRUE, otherwise the group can
 * not be waited on
 */
void dispatch_group_init(dispatch_group_t *group, bool waitable);

/** Free memory allocated by dispatch_group_create
 *
 * \param group  Group object
 */
void dispatch_group_delete(dispatch_group_t *group);

/** Creates a task and adds it to the the group
 *
 * \param group     Group object
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 *
 * \return          Task object
 */
dispatch_task_t *dispatch_group_function_add(dispatch_group_t *group,
                                             dispatch_function_t function,
                                             void *argument);

/** Add a task to the group
 *
 * \param group  Group object
 * \param task   Task to add
 */
void dispatch_group_task_add(dispatch_group_t *group, dispatch_task_t *task);

/** Run the group's tasks in the caller's thread
 *
 * \param group  Group object
 */
void dispatch_group_perform(dispatch_group_t *group);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // DISPATCH_GROUP_H_