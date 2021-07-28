// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_

#include "device_control.h"

#include "app_conf.h"

extern device_control_t *device_control_i2c_ctx;
extern device_control_t *device_control_usb_ctx;

control_ret_t app_control_servicer_register(device_control_servicer_t *ctx,
                                            const control_resid_t resources[],
                                            size_t num_resources);
int app_control_init(void);

#endif /* APP_CONTROL_H_ */
