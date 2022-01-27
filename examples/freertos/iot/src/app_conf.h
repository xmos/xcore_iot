// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

#define QSPI_RPC_PORT 14
#define QSPI_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES / 2 - 1)

/* Network Setup Configuration */
#define appconfSOFT_AP_SSID "xcore.ai"
#define appconfSOFT_AP_PASSWORD ""

/* MQTT demo defines */
#define appconfMQTT_PORT 8883
// If connecting to a MQTT broker on your PC, like mosquitto,
// appconfMQTT_HOSTNAME will likely be your IP address
#define appconfMQTT_HOSTNAME "your endpoint here"
#define appconfMQTT_CLIENT_ID "explorer"
#define appconfMQTT_DEMO_TOPIC "explorer/ledctrl"

/* Task Priorities */
#define appconfMQTT_TASK_PRIORITY (configMAX_PRIORITIES - 3)
#define appconfSNTPD_TASK_PRIORITY (configMAX_PRIORITIES - 2)
#define appconfSTARTUP_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfMEM_ANALYSIS_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfWIFI_SETUP_TASK_PRIORITY (configMAX_PRIORITIES / 2 - 1)
#define appconfWIFI_CONN_MNGR_TASK_PRIORITY (configMAX_PRIORITIES - 3)
#define appconfWIFI_DHCP_SERVER_TASK_PRIORITY (configMAX_PRIORITIES - 3)
#define appconfQSPI_FLASH_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define appconfSPI_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */
