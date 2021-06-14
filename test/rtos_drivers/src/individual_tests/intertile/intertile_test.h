// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef INTERTILE_TEST_H_
#define INTERTILE_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define intertile_printf( FMT, ... )       module_printf("INTERTILE", FMT, ##__VA_ARGS__)

#define INTERTILE_MAX_TESTS   2

#define INTERTILE_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_intertile_main_test_fptr_grp")))

typedef struct intertile_test_ctx intertile_test_ctx_t;

struct intertile_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[INTERTILE_MAX_TESTS];

    rtos_intertile_t *intertile_ctx;

    INTERTILE_MAIN_TEST_ATTR int (*main_test[INTERTILE_MAX_TESTS])(intertile_test_ctx_t *ctx);
};

typedef int (*intertile_main_test_t)(intertile_test_ctx_t *ctx);

int intertile_device_tests(rtos_intertile_t *intertile_ctx, chanend_t c);

/* Local Tests */
void register_fixed_len_tx_test(intertile_test_ctx_t *test_ctx);
void register_var_len_tx_test(intertile_test_ctx_t *test_ctx);

#endif /* INTERTILE_TEST_H_ */
