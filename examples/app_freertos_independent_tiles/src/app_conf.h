// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile device defines */
#define appconfINTERTILE_MAX_PIPES                  (4)
#define app_confINTERTILE_EVENT_QUEUE_LEN           (4)
#define appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS   (10)
#define appconfigNUM_INTERTILE_RX_DMA_BUF           (2)

/* Task Priorities */
#define appconfINTERTILE_CTRL_TASK_PRIORITY    ( configMAX_PRIORITIES - 3 )

#endif /* APP_CONF_H_ */
