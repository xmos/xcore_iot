// Copyright 2020-2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef APP_CONF_H_

#define PERSON_DETECT_PORT 13

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfPERSON_DETECT_TASK_PRIORITY      ( configMAX_PRIORITIES - 2 )
#define appconfSPI_CAMERA_TASK_PRIORITY    		( configMAX_PRIORITIES - 3 )

#endif /* APP_CONF_H_ */
