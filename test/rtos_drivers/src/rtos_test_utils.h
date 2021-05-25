// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_TEST_UTILS_H_
#define RTOS_TEST_UTILS_H_

typedef struct rtos_test_module {
    char *module_name;
    uint32_t num_tests;
    uint32_t log_level;
    void *app_data;
} rtos_test_module_t;

void rtos_test_init(rtos_test_module_t* ctx);
void rtos_test_register_test();

void rtos_test_print_impl();

#endif /* RTOS_TEST_UTILS_H_ */
