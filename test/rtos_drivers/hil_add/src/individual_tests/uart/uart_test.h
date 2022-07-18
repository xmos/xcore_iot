// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef UART_TEST_H_
#define UART_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define UART_RX_TILE 1
#define UART_TX_TILE 0

#define uart_printf( FMT, ... )       module_printf("UART", FMT, ##__VA_ARGS__)

#define UART_MAX_TESTS   1

#define UART_MAIN_TEST_ATTR __attribute__((fptrgroup("rtos_test_uart_main_test_fptr_grp")))
#define UART_RX_STARTED_ATTR        __attribute__((fptrgroup("rtos_test_uart_rx_started_fptr_grp")))
#define UART_RX_ERROR_ATTR  __attribute__((fptrgroup("rtos_test_uart_rx_error_fptr_grp")))
#define UART_RX_COMPLETE_ATTR   __attribute__((fptrgroup("rtos_test_uart_rx_complete_fptr_grp")))

typedef struct uart_test_ctx uart_test_ctx_t;

struct uart_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[UART_MAX_TESTS];

    rtos_uart_tx_t *rtos_uart_tx_ctx;
    rtos_uart_rx_t *rtos_uart_rx_ctx;
    int rx_success[UART_MAX_TESTS];

    UART_MAIN_TEST_ATTR int (*main_test[UART_MAX_TESTS])(uart_test_ctx_t *ctx);

    UART_RX_STARTED_ATTR void (*uart_rx_started[UART_MAX_TESTS])(rtos_uart_rx_t *ctx);
    UART_RX_ERROR_ATTR void (*uart_rx_error[UART_MAX_TESTS])(rtos_uart_rx_t *ctx, uint8_t err_flags);
    UART_RX_COMPLETE_ATTR void (*uart_rx_complete[UART_MAX_TESTS])(rtos_uart_rx_t *ctx);
};

typedef int (*uart_main_test_t)(uart_test_ctx_t *ctx);
typedef void (*uart_rx_started_t)(rtos_uart_rx_t *ctx);
typedef void (*uart_rx_error_t)(rtos_uart_rx_t *ctx, uint8_t err_flags);
typedef void (*uart_rx_complete_t)(rtos_uart_rx_t *ctx);

int uart_device_tests(rtos_uart_tx_t *rtos_uart_tx_ctx, rtos_uart_rx_t *rtos_uart_rx_ctx, chanend_t c);

// /* Local Tests */
void register_local_loopback_test(uart_test_ctx_t *test_ctx);

#endif /* UART_TEST_H_ */
