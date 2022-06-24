// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfUSB_MANAGER_TASK_PRIORITY        ( configMAX_PRIORITIES - 3 )
#define appconfTINYUSB_DEMO_TASK_PRIORITY       ( configMAX_PRIORITIES - 6 )

#endif /* APP_CONF_H_ */
