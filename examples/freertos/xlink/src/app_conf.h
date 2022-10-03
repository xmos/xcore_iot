// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* XLINK Configuration */
/* 0 = 2 wire; 1 = 5 wire */
#define appconfXLINK_WIRE_TYPE  0
#define appconfLINK_NUM  2
#define appconfINTER_DELAY 2
#define appconfINTRA_DELAY 3


#define appconfRX_DIRECTION 0
#define appconfRX_NODE_ID 0x20
#define appconfRX_DEBUG_I2C_SLAVE_ADDR 0xc
#define appconfRX_TIME_OUT_TICKS 500000000

#define appconfTX_DIRECTION 5
#define appconfRE_ENABLE_TX_PERIOD 6
#define appconfSEND_CTRL_TOKEN 2500000

/* Intertile Communication Configuration */
#define appconfI2C_MASTER_RPC_PORT 10
#define appconfI2C_MASTER_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfGPIO_T0_RPC_PORT 11
#define appconfGPIO_T1_RPC_PORT 12
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)

/* I/O and interrupt cores for Tile 0 */
#define appconfI2C_IO_CORE                      3 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfXLINK_RX_IO_CORE     1  /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfXLINK_TX_IO_CORE     1  /* Must be kept off core 0 with the RTOS tick ISR */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfGPIO_TASK_PRIORITY               ( configMAX_PRIORITIES - 2 )
#define appconfXLINK_RX_TASK_PRIORITY           ( configMAX_PRIORITIES - 1 )
#define appconfXLINK_TX_TASK_PRIORITY           ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
