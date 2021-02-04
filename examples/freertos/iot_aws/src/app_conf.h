// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* MQTT demo defines */
#define appconfMQTT_PORT						8883
#define appconfMQTT_HOSTNAME					"your endpoint"
#define appconfMQTT_CLIENT_ID					"ExplorerBoard"
#define appconfMQTT_DEMO_TOPIC					"explorer/ledctrl"

/* Task Priorities */
#define appconfMQTT_TASK_PRIORITY    			( configMAX_PRIORITIES - 3 )
#define appconfSNTPD_TASK_PRIORITY				( configMAX_PRIORITIES - 2 )

#endif /* APP_CONF_H_ */
