/*
 * app_conf.h
 *
 *  Created on: Oct 23, 2019
 *      Author: jmccarthy
 */


#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Audio Pipeline defines */
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN   42
#define appconfMIC_FRAME_LENGTH                256

/* Queue to TCP defines */
#define appconfQUEUE_TO_TCP_PORT                54321
#define appconfQUEUE_TO_TCP_ADDR0               10
#define appconfQUEUE_TO_TCP_ADDR1               129
#define appconfQUEUE_TO_TCP_ADDR2               28
#define appconfQUEUE_TO_TCP_ADDR3               119//mike:119 //jerry:238

//#define appconfQUEUE_TO_TCP_ADDR0               10
//#define appconfQUEUE_TO_TCP_ADDR1               128
//#define appconfQUEUE_TO_TCP_ADDR2               28
//#define appconfQUEUE_TO_TCP_ADDR3               187

//#define appconfQUEUE_TO_TCP_ADDR0               10
//#define appconfQUEUE_TO_TCP_ADDR1               128
//#define appconfQUEUE_TO_TCP_ADDR2               28
//#define appconfQUEUE_TO_TCP_ADDR3               18

#define DEBUG_PRINT_ENABLE_QUEUE_TO_TCP         0

#define appconfNETWORK_STATUS_CHECK_INTERVAL_MS     10
#define appconfNETWORK_GET_SOCKET_INTERVAL_MS       10
#define appconfNETWORK_CONNECT_RETRY_INTERVAL_MS    2000
#define appconfNETWORK_CONNECT_RETRY_LIMIT          2

/* CLI defines */
#define appconfCLI_UDP_PORT                     5432
#define configCOMMAND_INT_MAX_OUTPUT_SIZE       128

/* SDRAM test defines */
#define DEBUG_PRINT_ENABLE_SDRAM_TEST           0

/* Thruput test defines */
#define appconfTHRUPUT_TEST_PORT            10000
#define DEBUG_PRINT_ENABLE_THRUPUT_TEST         1

/* Task Priorities */
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    ( configMAX_PRIORITIES - 3 )
#define appconfQUEUE_TO_TCP_TASK_PRIORITY      ( configMAX_PRIORITIES - 2 )
#define appconfCLI_TASK_PRIORITY               ( configMAX_PRIORITIES - 3 )
#define appconfSDRAM_TEST_TASK_PRIORITY        ( configMAX_PRIORITIES - 3 )
#define appconfTHRUPUT_TEST_TASK_PRIORITY      ( configMAX_PRIORITIES - 3 )


#endif /* APP_CONF_H_ */
