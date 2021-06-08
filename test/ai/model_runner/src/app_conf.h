// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfMODEL_RUNNER_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfQSPI_FLASH_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfSWMEM_TASK_PRIORITY (configMAX_PRIORITIES - 1)

/* Model Runner */
#define appconfTENSOR_ARENA_SIZE                                               \
  (100000) // this is big enough for all test models
#define appconfMODEL_RUNNER_TASK_STACK_SIZE (1024)

/* ISR Dispatcher */
#define appconfDISPATCHER_CORE_MAP (0b00011111)

/* Thread Dispatcher */
#define appconfDISPATCHER_LENGTH (5)
#define appconfDISPATCHER_THREAD_COUNT (5)
#define appconfDISPATCHER_THREAD_PRIORITY (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */