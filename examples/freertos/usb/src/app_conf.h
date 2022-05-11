// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define appconfGPIO_T0_RPC_PORT 11
#define appconfGPIO_T1_RPC_PORT 12
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off I/O cores. Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           2 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfRTOS_QSPI_FLASH_TASK_PRIORITY    ( configMAX_PRIORITIES - 1 )
#define appconfUSB_MANAGER_TASK_PRIORITY        ( configMAX_PRIORITIES - 3 )
#define appconfTINYUSB_DEMO_TASK_PRIORITY       ( configMAX_PRIORITIES - 6 )

#endif /* APP_CONF_H_ */
