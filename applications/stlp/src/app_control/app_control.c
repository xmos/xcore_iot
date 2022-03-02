// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

/* Library headers */
#include "rtos_printf.h"
#include "device_control_usb.h"

/* App headers */
#include "app_conf.h"
#include "app_control.h"
#include "platform/driver_instances.h"

#define APP_CONTROL_TRANSPORT_COUNT ((appconfI2C_CTRL_ENABLED ? 1 : 0) + (appconfUSB_ENABLED ? 1 : 0))

#if ON_TILE(USB_TILE_NO)
static device_control_t device_control_usb_ctx_s;
#else
static device_control_client_t device_control_usb_ctx_s;
#endif
device_control_t *device_control_usb_ctx = (device_control_t *) &device_control_usb_ctx_s;

#if ON_TILE(I2C_CTRL_TILE_NO)
static device_control_t device_control_i2c_ctx_s;
#else
static device_control_client_t device_control_i2c_ctx_s;
#endif
device_control_t *device_control_i2c_ctx = (device_control_t *) &device_control_i2c_ctx_s;

static device_control_t *device_control_ctxs[APP_CONTROL_TRANSPORT_COUNT] = {
#if appconfUSB_ENABLED
        (device_control_t *) &device_control_usb_ctx_s,
#endif
#if appconfI2C_CTRL_ENABLED
        (device_control_t *) &device_control_i2c_ctx_s,
#endif
};

device_control_t *device_control_usb_get_ctrl_ctx_cb(void)
{
    return device_control_usb_ctx;
}

usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t *driver_count)
{
    *driver_count = 1;
    return &device_control_usb_app_driver;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    return device_control_usb_xfer(rhport, stage, request);
}

control_ret_t app_control_servicer_register(device_control_servicer_t *ctx,
                                            const control_resid_t resources[],
                                            size_t num_resources)
{
    return device_control_servicer_register(ctx,
                                            device_control_ctxs,
                                            APP_CONTROL_TRANSPORT_COUNT,
                                            resources, num_resources);
}

int app_control_init(void)
{
    control_ret_t ret = CONTROL_SUCCESS;

#if appconfI2C_CTRL_ENABLED
    if (ret == CONTROL_SUCCESS) {
        ret = device_control_init(device_control_i2c_ctx,
                                  THIS_XCORE_TILE == I2C_CTRL_TILE_NO ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                                  appconf_CONTROL_SERVICER_COUNT,
                                  &intertile_ctx, 1);
    }
    if (ret == CONTROL_SUCCESS) {
        ret = device_control_start(device_control_i2c_ctx,
                                   appconfDEVICE_CONTROL_I2C_PORT,
                                   appconfDEVICE_CONTROL_I2C_CLIENT_PRIORITY);
    }
#endif

#if appconfUSB_ENABLED
    if (ret == CONTROL_SUCCESS) {
        ret = device_control_init(device_control_usb_ctx,
                                  THIS_XCORE_TILE == USB_TILE_NO ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                                  appconf_CONTROL_SERVICER_COUNT,
                                  &intertile_ctx, 1);
    }
    if (ret == CONTROL_SUCCESS) {
        ret = device_control_start(device_control_usb_ctx,
                                   appconfDEVICE_CONTROL_USB_PORT,
                                   appconfDEVICE_CONTROL_USB_CLIENT_PRIORITY);
    }
#endif

    return ret;
}
