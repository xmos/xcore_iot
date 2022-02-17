// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef USB_TEST_H_
#define USB_TEST_H_

#include "rtos_test/rtos_test_utils.h"
#include "rtos_usb.h"

#define usb_printf( FMT, ... )       module_printf("USB", FMT, ##__VA_ARGS__)

#define USB_MAX_TESTS   1

#define USB_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_usb_main_test_fptr_grp")))

typedef struct usb_test_ctx usb_test_ctx_t;

struct usb_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[USB_MAX_TESTS];

    USB_MAIN_TEST_ATTR int (*main_test[USB_MAX_TESTS])(usb_test_ctx_t *ctx);
};

typedef int (*usb_main_test_t)(usb_test_ctx_t *ctx);

int usb_device_tests(chanend_t c);

/* Local Tests */
void register_uac_loopback_test(usb_test_ctx_t *test_ctx);

#endif /* USB_TEST_H_ */
