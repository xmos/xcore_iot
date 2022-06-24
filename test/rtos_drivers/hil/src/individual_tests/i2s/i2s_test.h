// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef I2S_TEST_H_
#define I2S_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define i2s_printf( FMT, ... )       module_printf("I2S", FMT, ##__VA_ARGS__)

#define I2S_MAX_TESTS   4

#define I2S_MAIN_TEST_ATTR __attribute__((fptrgroup("rtos_test_i2s_main_test_fptr_grp")))

typedef struct i2s_test_ctx i2s_test_ctx_t;

struct i2s_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[I2S_MAX_TESTS];

    rtos_i2s_t *i2s_master_ctx;
    rtos_i2s_t *i2s_slave_ctx;
    int slave_success[I2S_MAX_TESTS];

    I2S_MAIN_TEST_ATTR int (*main_test[I2S_MAX_TESTS])(i2s_test_ctx_t *ctx);
};

typedef int (*i2s_main_test_t)(i2s_test_ctx_t *ctx);

int i2s_device_tests(rtos_i2s_t *i2s_master_ctx, rtos_i2s_t *i2s_slave_ctx, chanend_t c);

/* Local Tests */
void register_master_to_slave_test(i2s_test_ctx_t *test_ctx);
void register_slave_to_master_test(i2s_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_master_to_slave_test(i2s_test_ctx_t *test_ctx);
void register_rpc_slave_to_master_test(i2s_test_ctx_t *test_ctx);

#endif /* I2S_TEST_H_ */
