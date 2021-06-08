// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#ifndef DISPATCH_GROUP_H_
#define DISPATCH_GROUP_H_

#include <stdbool.h>
#include <stddef.h>

#include "dispatch_job.h"

typedef struct dispatch_group_struct dispatch_group_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Create a new job group
 *
 * \param length    Maximum number of jobs in the group
 *
 * \return          Group object
 */
dispatch_group_t *dispatch_group_create(size_t length);

/** Initialize a new job group
 *
 * \param group     Group object
 */
void dispatch_group_init(dispatch_group_t *group);

/** Get pointer to the group's job array
 *
 * \param group     Group object
 *
 * \return          Task array pointer
 */
dispatch_job_t **dispatch_group_jobs_get(dispatch_group_t *group);

/** Free memory allocated by dispatch_group_create
 *
 * \param group  Group object
 */
void dispatch_group_delete(dispatch_group_t *group);

/** Creates a job and add it to the the group
 *
 * \param group     Group object
 * \param function  Function to perform, signature must be <tt>void(void*)</tt>
 * \param argument  Function argument
 *
 * \return          Task object
 */
dispatch_job_t *dispatch_group_function_add(dispatch_group_t *group,
                                            dispatch_function_t function,
                                            void *argument);

/** Add a job to the group
 *
 * \param group  Group object
 * \param job    Job to add
 */
void dispatch_group_job_add(dispatch_group_t *group, dispatch_job_t *job);

/** Run the group's jobs in the caller's thread
 *
 * \param group  Group object
 */
void dispatch_group_perform(dispatch_group_t *group);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // DISPATCH_GROUP_H_