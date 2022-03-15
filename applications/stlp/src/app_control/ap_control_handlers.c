// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <string.h>
#include "app_control.h"

#define APP_CONTROL_CMD_AP_VERSION 0x00
#define APP_CONTROL_CMD_AP_MIC_FROM_USB 0x01

static device_control_servicer_t ap_servicer;

extern volatile int mic_from_usb;
extern volatile int aec_ref_source;

static int32_t fixed_point_vals[2];

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t ap_read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    control_ret_t ret = CONTROL_SUCCESS;

    switch (cmd) {
    case CONTROL_CMD_SET_READ(APP_CONTROL_CMD_AP_VERSION):
        if (payload_len == sizeof(uint32_t)) {
            *((uint32_t *) payload) = 12345678;
        } else {
            ret = CONTROL_DATA_LENGTH_ERROR;
        }
        break;
    case CONTROL_CMD_SET_READ(APP_CONTROL_CMD_AP_MIC_FROM_USB):
        if (payload_len == sizeof(uint8_t)) {
            *payload = mic_from_usb;
        } else {
            ret = CONTROL_DATA_LENGTH_ERROR;
        }
        break;
    case CONTROL_CMD_SET_READ(0x7F):
        if (payload_len == 2 * sizeof(int32_t)) {
            ((int32_t *) payload)[0] = fixed_point_vals[0];
            ((int32_t *) payload)[1] = fixed_point_vals[1];
        } else {
            ret = CONTROL_DATA_LENGTH_ERROR;
        }
        break;
    default:
        rtos_printf("Audio pipeline control read command %02x\n", cmd);
        memset(payload, 0, payload_len);
        ret = CONTROL_BAD_COMMAND;
    }

    return ret;
}

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t ap_write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    control_ret_t ret = CONTROL_SUCCESS;

    switch (cmd) {
    case CONTROL_CMD_SET_WRITE(APP_CONTROL_CMD_AP_MIC_FROM_USB):
        if (payload_len == sizeof(uint8_t)) {
            mic_from_usb = *payload;
        } else {
            ret = CONTROL_DATA_LENGTH_ERROR;
        }
        break;
    case CONTROL_CMD_SET_WRITE(0x7F):
        if (payload_len == 2 * sizeof(int32_t)) {
            fixed_point_vals[0] = ((int32_t *) payload)[0];
            fixed_point_vals[1] = ((int32_t *) payload)[1];
        } else {
            ret = CONTROL_DATA_LENGTH_ERROR;
        }
        break;
    default:
        rtos_printf("Audio pipeline control write command %02x\n", cmd);
        ret = CONTROL_BAD_COMMAND;
    }

    return ret;
}

void app_control_ap_handler(void *state, unsigned timeout)
{
    device_control_servicer_cmd_recv(&ap_servicer, ap_read_cmd, ap_write_cmd, state, timeout);
}

void app_control_ap_servicer_register(void)
{
    const control_resid_t resources[] = {APP_CONTROL_RESID_AP};
    control_ret_t dc_ret;

    rtos_printf("Will register the AP servicer now\n");
    dc_ret = app_control_servicer_register(&ap_servicer,
                                           resources, sizeof(resources));
    xassert(dc_ret == CONTROL_SUCCESS);
    rtos_printf("AP servicer registered\n");
}
