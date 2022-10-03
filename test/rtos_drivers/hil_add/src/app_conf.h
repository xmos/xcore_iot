// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define INTERTILE_RPC_PORT 10
#define INTERTILE_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define SPI_MASTER_RPC_PORT 11
#define SPI_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define SPI_TEST_CPOL   0
#define SPI_TEST_CPHA   0

#define SPI_SLAVE_CORE_MASK     (1 << 4)
#define SPI_SLAVE_ISR_CORE      5

#define SPI_TEST_BUF_SIZE       4096

#define UART_RX_CORE_MASK       (1 << 2)
#define UART_RX_ISR_CORE        2
#define UART_BAUD_RATE          921600


/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
