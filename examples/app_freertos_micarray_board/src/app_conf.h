// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Audio Pipeline defines */
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN   42
#define appconfMIC_FRAME_LENGTH                256

/* Queue to TCP defines */
#define appconfQUEUE_TO_TCP_PORT                54321

#define DEBUG_PRINT_ENABLE_QUEUE_TO_TCP         0

#define appconfNETWORK_STATUS_CHECK_INTERVAL_MS     10
#define appconfNETWORK_GET_SOCKET_INTERVAL_MS       10
#define appconfNETWORK_CONNECT_RETRY_INTERVAL_MS    2000
#define appconfNETWORK_CONNECT_RETRY_LIMIT          2

/* CLI defines */
#define appconfCLI_UDP_PORT                     5432
#define configCOMMAND_INT_MAX_OUTPUT_SIZE       128

/* Thruput test defines */
#define appconfTHRUPUT_TEST_PORT            10000
#define DEBUG_PRINT_ENABLE_THRUPUT_TEST         1

/* GPIO defines */
#define appconfGPIO_VOLUME_RAPID_FIRE_MS       100

/* SPI Test Defines */
/* Setup for mode 0 */
#define appconfSPI_CS_PORT_BIT  0
#define appconfSPI_CPOL         0
#define appconfSPI_CPHA         0

/* 100 MHz / (2 * 7) / 2 = 3.57 MHz SCK. aardvark slave max br is 4MHz */
#define appconfSPI_CLOCKDIV     7

/* tsu_cs on WF200 is 3 ns. aardvark is 10000 (10 us) */
#define appconfSPI_CS_TO_DATA_DELAY_NS  10000

/* td on WF200 is 0 ns. aardvark is 4000 (4 us) */
#define appconfSPI_BYTE_SETUP_NS    4000


/* Task Priorities */
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    ( configMAX_PRIORITIES - 3 )
#define appconfQUEUE_TO_TCP_TASK_PRIORITY      ( configMAX_PRIORITIES - 2 )
#define appconfQUEUE_TO_I2S_TASK_PRIORITY      ( configMAX_PRIORITIES - 2 )
#define appconfCLI_TASK_PRIORITY               ( configMAX_PRIORITIES - 3 )
#define appconfTHRUPUT_TEST_TASK_PRIORITY      ( configMAX_PRIORITIES - 3 )
#define appconfGPIO_TASK_PRIORITY              ( configMAX_PRIORITIES - 2 )

#endif /* APP_CONF_H_ */
