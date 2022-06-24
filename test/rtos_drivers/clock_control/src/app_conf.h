// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

#define appconfCLOCK_CONTROL_PORT      7

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY              (configMAX_PRIORITIES/2 + 5)
#define appconfCLOCK_CONTROL_RPC_HOST_PRIORITY    (configMAX_PRIORITIES/2 + 2)

#endif /* APP_CONF_H_ */
