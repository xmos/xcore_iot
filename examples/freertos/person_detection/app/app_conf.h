// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_

#define PERSON_DETECT_PORT 13

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfPERSON_DETECT_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfSPI_CAMERA_TASK_PRIORITY (configMAX_PRIORITIES - 3)

/* Thread Dispatcher */
#define appconfDISPATCHER_LENGTH (5)
#define appconfDISPATCHER_THREAD_COUNT (5)
#define appconfDISPATCHER_THREAD_PRIORITY (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */
