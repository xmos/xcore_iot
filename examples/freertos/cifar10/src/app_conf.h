// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_

#define CIFAR10_PORT 13

#define QSPI_RPC_PORT 14
#define QSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2 - 1)

/* Task Priorities */
#define appconfQSPI_FLASH_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfCIFAR10_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfSWMEM_TASK_PRIORITY (configMAX_PRIORITIES - 1)

/* Thread Dispatcher */
#define appconfDISPATCHER_LENGTH (5)
#define appconfDISPATCHER_THREAD_COUNT (5)
#define appconfDISPATCHER_THREAD_PRIORITY (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */
