// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef UART_TEST_H_
#define UART_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define uart_printf( FMT, ... )       module_printf("UART", FMT, ##__VA_ARGS__)

#define UART_MAX_TESTS   1

// #define SPI_MAIN_TEST_ATTR          __attribute__((fptrgroup("rtos_test_spi_main_test_fptr_grp")))
// #define SPI_SLAVE_XFER_DONE_ATTR    __attribute__((fptrgroup("rtos_test_spi_slave_xfer_done_fptr_grp")))

typedef struct uart_test_ctx uart_test_ctx_t;

struct uart_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[UART_MAX_TESTS];

    rtos_uart_tx_t *rtos_uart_tx_ctx;
    rtos_uart_rx_t *rtos_uart_rx_ctx;
    int rx_success[UART_MAX_TESTS];

};

int uart_device_tests(rtos_uart_tx_t *rtos_uart_tx_ctx, rtos_uart_rx_t *rtos_uart_rx_ctx);

// /* Local Tests */
void register_local_loopback_test(uart_test_ctx_t *test_ctx);

#endif /* UART_TEST_H_ */
