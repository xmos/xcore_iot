// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <string.h>
#include "app_control.h"

static device_control_servicer_t stage2_servicer;

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t stage2_read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Stage 2 control read command %02x\n", cmd);

    memset(payload, 0, payload_len);

    return CONTROL_SUCCESS;
}

DEVICE_CONTROL_CALLBACK_ATTR
static control_ret_t stage2_write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Stage 2 control write command %02x\n", cmd);

    return CONTROL_SUCCESS;
}

void app_control_stage2_handler(void *state, unsigned timeout)
{
    device_control_servicer_cmd_recv(&stage2_servicer, stage2_read_cmd, stage2_write_cmd, state, timeout);
}

void app_control_stage2_servicer_register(void)
{
    const control_resid_t resources[] = {APP_CONTROL_RESID_STAGE2};
    control_ret_t dc_ret;

    rtos_printf("Will register the Stage 2 servicer now\n");
    dc_ret = app_control_servicer_register(&stage2_servicer,
                                           resources, sizeof(resources));
    xassert(dc_ret == CONTROL_SUCCESS);
    rtos_printf("Stage 2 servicer registered\n");
}
