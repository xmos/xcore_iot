// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MIC_ARRAY_TEST_H_
#define MIC_ARRAY_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define mic_array_printf( FMT, ... )       module_printf("MIC_ARRAY", FMT, ##__VA_ARGS__)

#define MIC_ARRAY_MAX_TESTS   2

#define MIC_ARRAY_MAIN_TEST_ATTR __attribute__((fptrgroup("rtos_test_mic_array_main_test_fptr_grp")))

typedef struct mic_array_test_ctx mic_array_test_ctx_t;

struct mic_array_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[MIC_ARRAY_MAX_TESTS];

    rtos_mic_array_t *mic_array_ctx;
    int slave_success[MIC_ARRAY_MAX_TESTS];

    MIC_ARRAY_MAIN_TEST_ATTR int (*main_test[MIC_ARRAY_MAX_TESTS])(mic_array_test_ctx_t *ctx);
};

typedef int (*mic_array_main_test_t)(mic_array_test_ctx_t *ctx);

int mic_array_device_tests(rtos_mic_array_t *mic_array_ctx, chanend_t c);

/* Local Tests */
void register_get_samples_test(mic_array_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_get_samples_test(mic_array_test_ctx_t *test_ctx);

#endif /* MIC_ARRAY_TEST_H_ */
