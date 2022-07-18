// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef GPIO_TEST_H_
#define GPIO_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define gpio_printf( FMT, ... )       module_printf("GPIO", FMT, ##__VA_ARGS__)

#define GPIO_MAX_TESTS   2

#define GPIO_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_gpio_main_test_fptr_grp")))

typedef struct gpio_test_ctx gpio_test_ctx_t;

struct gpio_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[GPIO_MAX_TESTS];

    rtos_gpio_t *gpio_ctx;

    GPIO_MAIN_TEST_ATTR int (*main_test[GPIO_MAX_TESTS])(gpio_test_ctx_t *ctx);
};

typedef int (*gpio_main_test_t)(gpio_test_ctx_t *ctx);

#define INPUT_PORT              XS1_PORT_1P
#define INPUT_PORT_PIN_OFFSET   0

#define OUTPUT_PORT             XS1_PORT_4A
#define OUTPUT_PORT_PIN_OFFSET  0

int gpio_device_tests(rtos_gpio_t *gpio_ctx, chanend_t c);

/* Local Tests */
void register_io_test(gpio_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_io_test(gpio_test_ctx_t *test_ctx);

#endif /* GPIO_TEST_H_ */
