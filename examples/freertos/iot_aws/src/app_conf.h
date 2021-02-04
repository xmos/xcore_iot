// Copyright (c) 2020-2021, XMOS Ltd, All rights reserved

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Network Setup Configuration */
#define appconfSOFT_AP_SSID         "xcore.ai"
#define appconfSOFT_AP_PASSWORD     ""

/* MQTT demo defines */
#define appconfMQTT_PORT						8883
#define appconfMQTT_HOSTNAME					"your endpoint here"
#define appconfMQTT_CLIENT_ID					"ExplorerBoard"
#define appconfMQTT_DEMO_TOPIC					"explorer/ledctrl"

/* Task Priorities */
#define appconfMQTT_TASK_PRIORITY    			( configMAX_PRIORITIES - 3 )
#define appconfSNTPD_TASK_PRIORITY				( configMAX_PRIORITIES - 2 )
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfMEM_ANALYSIS_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define appconfWIFI_SETUP_TASK_PRIORITY		    ( configMAX_PRIORITIES/2 - 1 )
#define appconfWIFI_CONN_MNGR_TASK_PRIORITY     ( configMAX_PRIORITIES - 3 )
#define appconfWIFI_DHCP_SERVER_TASK_PRIORITY   ( configMAX_PRIORITIES - 3 )

#endif /* APP_CONF_H_ */
