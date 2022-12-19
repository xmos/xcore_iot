// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define appconfI2C_MASTER_RPC_PORT 10
#define appconfI2C_MASTER_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfGPIO_T0_RPC_PORT 11
#define appconfGPIO_T1_RPC_PORT 12
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)

/* I/O and interrupt cores for Tile 0 */
#define appconfI2C_IO_CORE                      4 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */
#define appconfXUD_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfUSB_INTERRUPT_CORE               3 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfUSB_SOF_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off cores with other ISRs. */
#define appconfSPI_IO_CORE                      4 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfSPI_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfUART_RX_IO_CORE                  3 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfUART_RX_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */

/* UART Configuration */
#define appconfUART_BAUD_RATE                   806400

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfSPI_MASTER_TASK_PRIORITY         ( configMAX_PRIORITIES - 1 )
#define appconfQSPI_FLASH_TASK_PRIORITY         ( configMAX_PRIORITIES - 1 )
#define appconfUART_RX_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfUSB_MANAGER_TASK_PRIORITY        ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
