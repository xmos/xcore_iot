// Copyright (c) 2020-2021, XMOS Ltd, All rights reserved

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile communication defines */
#define I2C_MASTER_RPC_PORT 10
#define I2C_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define GPIO_RPC_PORT 11
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define appconfAUDIO_CLOCK_FREQUENCY    24576000
#define appconfPDM_CLOCK_FREQUENCY      3072000

/* Audio Pipeline defines */
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN   	42
#define appconfMIC_FRAME_LENGTH                	256
// #define appconfPRINT_AUDIO_FRAME_POWER         	1

/* Queue to TCP defines */
#define appconfQUEUE_TO_TCP_PORT                54321
#define appconfTCP_TO_QUEUE_PORT                12345

#define DEBUG_PRINT_ENABLE_QUEUE_TO_TCP         0

#define appconfNETWORK_STATUS_CHECK_INTERVAL_MS     10
#define appconfNETWORK_GET_SOCKET_INTERVAL_MS       10
#define appconfNETWORK_CONNECT_RETRY_INTERVAL_MS	2000
#define appconfNETWORK_CONNECT_RETRY_LIMIT          2

/* CLI defines */
#define appconfCLI_UDP_PORT                     5432
#define configCOMMAND_INT_MAX_OUTPUT_SIZE       128

/* Thruput test defines */
#define appconfTHRUPUT_TEST_PORT            	10000
#define DEBUG_PRINT_ENABLE_THRUPUT_TEST         1

/* GPIO defines */
#define appconfGPIO_VOLUME_RAPID_FIRE_MS       	100

/* Echo demo defines */
#define appconfECHO_IP_ADDR_OCTET_0    	10
#define appconfECHO_IP_ADDR_OCTET_1    	0
#define appconfECHO_IP_ADDR_OCTET_2    	0
#define appconfECHO_IP_ADDR_OCTET_3    	253
#define appconfECHO_PORT				25565

/* MQTT demo defines */
#define appconfMQTT_SERVER_IP_ADDR_OCTET_0    	10
#define appconfMQTT_SERVER_IP_ADDR_OCTET_1    	0
#define appconfMQTT_SERVER_IP_ADDR_OCTET_2    	0
#define appconfMQTT_SERVER_IP_ADDR_OCTET_3    	253
#define appconfMQTT_PORT						8883

/* Task Priorities */
#define appconfMQTT_TASK_PRIORITY    			( configMAX_PRIORITIES - 3 )
#define appconfHTTP_TASK_PRIORITY    			( configMAX_PRIORITIES - 3 )
#define appconfTLS_ECHO_TASK_PRIORITY    		( configMAX_PRIORITIES - 3 )
#define appconfTLS_ECHO_SERVER_PRIORITY    		( configMAX_PRIORITIES - 3 )
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    	( configMAX_PRIORITIES - 3 )
#define appconfTCP_TO_QUEUE_TASK_PRIORITY    	( configMAX_PRIORITIES - 3 )
#define appconfQUEUE_TO_TCP_TASK_PRIORITY      	( configMAX_PRIORITIES - 2 )
#define appconfQUEUE_TO_I2S_TASK_PRIORITY      	( configMAX_PRIORITIES - 2 )
#define appconfCLI_TASK_PRIORITY               	( configMAX_PRIORITIES - 3 )
#define appconfTHRUPUT_TEST_TASK_PRIORITY      	( configMAX_PRIORITIES - 3 )
#define appconfGPIO_TASK_PRIORITY              	( configMAX_PRIORITIES - 2 )
#define appconfSNTPD_TASK_PRIORITY				( configMAX_PRIORITIES - 5 )

#endif /* APP_CONF_H_ */
