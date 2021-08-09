// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define appconfI2C_MASTER_RPC_PORT 10
#define appconfI2C_MASTER_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfGPIO_RPC_PORT 11
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfINTERTILE_AUDIOPIPELINE_PORT 12
#define appconfINTERTILE_AUDIOPIPELINE_TASK_PRIORITY (configMAX_PRIORITIES-2)

#define appconfCLI_RPC_PROCESS_COMMAND_PORT 13
#define appconfCLI_RPC_PROCESS_COMMAND_TASK_PRIORITY (configMAX_PRIORITIES/2)


/* I/O and interrupt cores for Tile 0 */
#define appconfI2C_IO_CORE                      3 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2S_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           3 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2S_INTERRUPT_CORE               4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */

/* Network Demo Configuration */
#define appconfSOFT_AP_SSID         "xcore.ai"
#define appconfSOFT_AP_PASSWORD     ""

#define appconfRUN_PING_TEST_HOST   1
#define appconfPING_TEST_HOST       "google.com"

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            24576000
#define appconfPDM_CLOCK_FREQUENCY              3072000
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       16000
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN    42
#define appconfAUDIO_FRAME_LENGTH            	256
#define appconfPRINT_AUDIO_FRAME_POWER          0

/* Queue to TCP Configuration */
#define appconfQUEUE_TO_TCP_PORT                54321
#define appconfTCP_TO_QUEUE_PORT                12345

#define DEBUG_PRINT_ENABLE_QUEUE_TO_TCP         0

#define appconfNETWORK_STATUS_CHECK_INTERVAL_MS     10
#define appconfNETWORK_GET_SOCKET_INTERVAL_MS       10
#define appconfNETWORK_CONNECT_RETRY_INTERVAL_MS	2000
#define appconfNETWORK_CONNECT_RETRY_LIMIT          2

/* CLI Configuration */
#define appconfCLI_UDP_PORT                     5432
#define configCOMMAND_INT_MAX_OUTPUT_SIZE       128

/* Thruput Test Configuration */
#define appconfTHRUPUT_TEST_PORT            	10000
#define DEBUG_PRINT_ENABLE_THRUPUT_TEST         1

/* GPIO Configuration */
#define appconfGPIO_VOLUME_RAPID_FIRE_MS       	100

/* Echo Demo Configuration */
#define appconfECHO_IP_ADDR_OCTET_0    	10
#define appconfECHO_IP_ADDR_OCTET_1    	0
#define appconfECHO_IP_ADDR_OCTET_2    	0
#define appconfECHO_IP_ADDR_OCTET_3    	253
#define appconfECHO_PORT				25565

/* MQTT Demo Configuration */
#define appconfMQTT_SERVER_IP_ADDR_OCTET_0    	10
#define appconfMQTT_SERVER_IP_ADDR_OCTET_1    	0
#define appconfMQTT_SERVER_IP_ADDR_OCTET_2    	0
#define appconfMQTT_SERVER_IP_ADDR_OCTET_3    	253
#define appconfMQTT_PORT						8883

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfMQTT_TASK_PRIORITY    			( configMAX_PRIORITIES - 7 )
#define appconfHTTP_TASK_PRIORITY    			( configMAX_PRIORITIES - 8 )
#define appconfTLS_ECHO_TASK_PRIORITY    		( configMAX_PRIORITIES - 8 )
#define appconfTLS_ECHO_SERVER_PRIORITY    		( configMAX_PRIORITIES - 8 )
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    	( configMAX_PRIORITIES - 4 )
#define appconfQUEUE_TO_TCP_TASK_PRIORITY      	( configMAX_PRIORITIES - 3 )
#define appconfCLI_TASK_PRIORITY               	( configMAX_PRIORITIES - 5 )
#define appconfTHRUPUT_TEST_TASK_PRIORITY      	( configMAX_PRIORITIES - 3 )
#define appconfGPIO_TASK_PRIORITY              	( configMAX_PRIORITIES - 2 )
#define appconfSNTPD_TASK_PRIORITY				( configMAX_PRIORITIES - 5 )
#define appconfMEM_ANALYSIS_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define appconfWIFI_SETUP_TASK_PRIORITY		    ( configMAX_PRIORITIES/2 - 1 )
#define appconfWIFI_CONN_MNGR_TASK_PRIORITY     ( configMAX_PRIORITIES - 3 )
#define appconfWIFI_DHCP_SERVER_TASK_PRIORITY   ( configMAX_PRIORITIES - 3 )
#define appconfSPI_MASTER_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )
#define appconfQSPI_FLASH_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )













#endif /* APP_CONF_H_ */
