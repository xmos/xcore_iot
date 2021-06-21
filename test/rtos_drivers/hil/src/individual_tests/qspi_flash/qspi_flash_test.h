// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef QSPI_FLASH_TEST_H_
#define QSPI_FLASH_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define qspi_flash_printf( FMT, ... )       module_printf("QSPI_FLASH", FMT, ##__VA_ARGS__)

#define QSPI_FLASH_MAX_TESTS   4

#define QSPI_FLASH_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_qspi_flash_main_test_fptr_grp")))

typedef struct qspi_flash_test_ctx qspi_flash_test_ctx_t;

struct qspi_flash_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[QSPI_FLASH_MAX_TESTS];

    rtos_qspi_flash_t *qspi_flash_ctx;

    QSPI_FLASH_MAIN_TEST_ATTR int (*main_test[QSPI_FLASH_MAX_TESTS])(qspi_flash_test_ctx_t *ctx);
};

typedef int (*qspi_flash_main_test_t)(qspi_flash_test_ctx_t *ctx);

int qspi_flash_device_tests(rtos_qspi_flash_t *qspi_flash_ctx, chanend_t c);

/* Independant Tests */
void register_check_params_test(qspi_flash_test_ctx_t *test_ctx);
void register_multiple_user_test(qspi_flash_test_ctx_t *test_ctx);

/* Local Tests */
void register_read_write_read_test(qspi_flash_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_read_write_read_test(qspi_flash_test_ctx_t *test_ctx);

#endif /* QSPI_FLASH_TEST_H_ */
