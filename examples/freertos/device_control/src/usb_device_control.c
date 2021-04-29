// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <stdlib.h>

#include "rtos_printf.h"
#include "device_control.h"
#include "tusb.h"
#include "device/usbd_pvt.h"

static device_control_t *device_control_ctx;
static size_t servicer_count_g;

#if CFG_TUSB_DEBUG >= 2
  #define DRIVER_NAME(_name)    .name = _name,
#else
  #define DRIVER_NAME(_name)
#endif

static void usb_device_control_init(void)
{
    rtos_printf("USB Device Control Driver Initialized!\n");
}

static void usb_device_control_reset(uint8_t rhport)
{
  (void) rhport;

  rtos_printf("USB Device Control Driver Reset!\n");
}

static uint16_t usb_device_control_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t max_len)
{
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass);

    TU_VERIFY(itf_desc->bNumEndpoints == 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len);

    control_ret_t dc_ret;

    TU_VERIFY(device_control_ctx != NULL);

    dc_ret = device_control_resources_register(device_control_ctx,
                                               servicer_count_g,
                                               pdMS_TO_TICKS(100));

    if (dc_ret != CONTROL_SUCCESS) {
        rtos_printf("Device control resources failed to register for USB on tile %d\n", THIS_XCORE_TILE);
    } else {
        rtos_printf("Device control resources registered for USB on tile %d\n", THIS_XCORE_TILE);
    }
    TU_VERIFY(dc_ret == CONTROL_SUCCESS);

    rtos_printf("Device control USB interface #%d opened\n", itf_desc->bInterfaceNumber);

    return drv_len;
}

static uint8_t request_data[CFG_TUD_ENDPOINT0_SIZE];


#define REQ_TYPE(direction, type, recipient) ((uint8_t) ((direction) << 7)) | ((uint8_t) ((type) << 5)) | (recipient)

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
    control_ret_t ret;
    size_t len;

    if (stage == CONTROL_STAGE_SETUP) {
        ret = device_control_request(device_control_ctx,
                                     request->wIndex,
                                     request->wValue,
                                     request->wLength);
        if (ret != CONTROL_SUCCESS) {
            rtos_printf("Bad command received: %02x, %02x, %d\n", request->wIndex, request->wValue, request->wLength);
            return false;
        }
    }

    switch (request->bmRequestType) {
    case REQ_TYPE(TUSB_DIR_OUT, TUSB_REQ_TYPE_VENDOR, TUSB_REQ_RCPT_DEVICE):

        if (stage == CONTROL_STAGE_SETUP) {
            return tud_control_xfer(rhport, request, request_data, request->wLength);
        } else if (stage == CONTROL_STAGE_DATA) {
            len = request->wLength;
            ret = device_control_payload_transfer(device_control_ctx,
                                                  request_data, &len,
                                                  CONTROL_HOST_TO_DEVICE);
            if (ret == CONTROL_SUCCESS) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }

    case REQ_TYPE(TUSB_DIR_IN, TUSB_REQ_TYPE_VENDOR, TUSB_REQ_RCPT_DEVICE):

        if (stage == CONTROL_STAGE_SETUP) {
            len = CFG_TUD_ENDPOINT0_SIZE;
            ret = device_control_payload_transfer(device_control_ctx,
                                                  request_data, &len,
                                                  CONTROL_DEVICE_TO_HOST);

            if (ret == CONTROL_SUCCESS) {
                return tud_control_xfer(rhport, request, request_data, len);
            } else {
                return false;
            }
        } else {
            return true;
        }
    }

    return false;
}

static const usbd_class_driver_t app_drv[1] = {
    {
        DRIVER_NAME("XMOS-DEVICE-CONTROL")
        .init = usb_device_control_init,
        .reset = usb_device_control_reset,
        .open = usb_device_control_open,
        .control_xfer_cb = NULL,
        .xfer_cb = NULL,
        .sof = NULL,
    }
};

void usb_device_control_set_ctx(device_control_t *ctx,
                                size_t servicer_count)
{
    device_control_ctx = ctx;
    servicer_count_g = servicer_count;
}

usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t* driver_count)
{
    *driver_count = 1;
    return app_drv;
}
