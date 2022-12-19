// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SPI_TEST_H_
#define SPI_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define spi_printf( FMT, ... )       module_printf("SPI", FMT, ##__VA_ARGS__)

#define SPI_MAX_TESTS   5

#define SPI_MAIN_TEST_ATTR          __attribute__((fptrgroup("rtos_test_spi_main_test_fptr_grp")))
#define SPI_SLAVE_XFER_DONE_ATTR    __attribute__((fptrgroup("rtos_test_spi_slave_xfer_done_fptr_grp")))

typedef struct spi_test_ctx spi_test_ctx_t;

struct spi_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[SPI_MAX_TESTS];

    rtos_spi_master_t *spi_master_ctx;
    rtos_spi_master_device_t *spi_device_ctx;
    rtos_spi_slave_t *spi_slave_ctx;
    int slave_success[SPI_MAX_TESTS];

    SPI_MAIN_TEST_ATTR int (*main_test[SPI_MAX_TESTS])(spi_test_ctx_t *ctx);
    SPI_SLAVE_XFER_DONE_ATTR int (*slave_xfer_done[SPI_MAX_TESTS])(rtos_spi_slave_t *ctx, void *app_data);
};

typedef int (*spi_main_test_t)(spi_test_ctx_t *ctx);
typedef int (*spi_slave_xfer_done_t)(rtos_spi_slave_t *ctx, void *app_data);

int spi_device_tests(rtos_spi_master_t *spi_master_ctx, rtos_spi_master_device_t *spi_device_ctx, rtos_spi_slave_t *spi_slave_ctx, chanend_t c);

/* Local Tests */
void register_single_transaction_test(spi_test_ctx_t *test_ctx);
void register_multiple_transaction_test(spi_test_ctx_t *test_ctx);
void register_slave_default_buffer_test(spi_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_single_transaction_test(spi_test_ctx_t *test_ctx);
void register_rpc_multiple_transaction_test(spi_test_ctx_t *test_ctx);

#endif /* SPI_TEST_H_ */
