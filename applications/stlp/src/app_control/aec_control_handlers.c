// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <string.h>
#include "app_control.h"

static device_control_servicer_t aec_servicer;

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t aec_read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("AEC control read command %02x\n", cmd);

    memset(payload, 0, payload_len);

    return CONTROL_SUCCESS;
}

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t aec_write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("AEC control write command %02x\n", cmd);

    return CONTROL_SUCCESS;
}

void app_control_aec_handler(void *state, unsigned timeout)
{
    device_control_servicer_cmd_recv(&aec_servicer, aec_read_cmd, aec_write_cmd, state, timeout);
}

void app_control_aec_servicer_register(void)
{
    const control_resid_t resources[] = {APP_CONTROL_RESID_AEC};
    control_ret_t dc_ret;

    rtos_printf("Will register the AEC servicer now\n");
    dc_ret = app_control_servicer_register(&aec_servicer,
                                           resources, sizeof(resources));
    xassert(dc_ret == CONTROL_SUCCESS);
    rtos_printf("AEC servicer registered\n");
}
