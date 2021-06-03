// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfMODEL_RUNNER_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfSWMEM_TASK_PRIORITY (configMAX_PRIORITIES - 2)

/* Model Runner */
#define appconfTENSOR_ARENA_SIZE \
  (100000)  // this is big enough for all test models
#define appconfMODEL_RUNNER_TASK_STACK_SIZE (1024)

/* Dispatch Queue */
#define appconfDISPATCH_QUEUE_LENGTH (5)
#define appconfDISPATCH_QUEUE_THREAD_COUNT (5)
#define appconfDISPATCH_QUEUE_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfDISPATCH_QUEUE_THREAD_STACK_SIZE (512)

#endif /* APP_CONF_H_ */