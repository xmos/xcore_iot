// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SWMEM_TEST_H_
#define SWMEM_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define swmem_printf( FMT, ... )       module_printf("SWMEM", FMT, ##__VA_ARGS__)

#define SWMEM_MAX_TESTS   1

#define SWMEM_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_swmem_main_test_fptr_grp")))

typedef struct swmem_test_ctx swmem_test_ctx_t;

struct swmem_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[SWMEM_MAX_TESTS];

    SWMEM_MAIN_TEST_ATTR int (*main_test[SWMEM_MAX_TESTS])(swmem_test_ctx_t *ctx);
};

typedef int (*swmem_main_test_t)(swmem_test_ctx_t *ctx);

int swmem_device_tests(chanend_t c);

/* Local Tests */
void register_swmem_read_write_test(swmem_test_ctx_t *test_ctx);

#endif /* SWMEM_TEST_H_ */
