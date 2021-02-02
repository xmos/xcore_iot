// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef APP_CONF_H_

#define CIFAR10_PORT 13

#define QSPI_RPC_PORT 14
#define QSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2 - 1)

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfCIFAR10_TASK_PRIORITY            ( configMAX_PRIORITIES - 2 )
#define appconfSWMEM_TASK_PRIORITY              ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
