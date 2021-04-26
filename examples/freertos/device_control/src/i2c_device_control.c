// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>

#include "rtos/drivers/i2c/api/rtos_i2c_slave.h"

#include "rtos_printf.h"
#include "device_control.h"

RTOS_I2C_SLAVE_CALLBACK_ATTR
void i2c_dev_ctrl_start_cb(rtos_i2c_slave_t *ctx,
                           device_control_t *device_control_ctx)
{
    control_ret_t dc_ret;

    dc_ret = device_control_resources_register(device_control_ctx,
                                               2, //SERVICER COUNT
                                               pdMS_TO_TICKS(100));

    if (dc_ret != CONTROL_SUCCESS) {
        rtos_printf("Device control resources failed to register for I2C on tile %d\n", THIS_XCORE_TILE);
    } else {
        rtos_printf("Device control resources registered for I2C on tile %d\n", THIS_XCORE_TILE);
    }
    xassert(dc_ret == CONTROL_SUCCESS);
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void i2c_dev_ctrl_rx_cb(rtos_i2c_slave_t *ctx,
                        device_control_t *device_control_ctx,
                        uint8_t *data,
                        size_t len)
{
    control_ret_t ret;

    if (len >= 3) {
        device_control_request(device_control_ctx,
                               data[0],
                               data[1],
                               data[2]);

        len -= 3;
        ret = device_control_payload_transfer(device_control_ctx,
                                              &data[3], &len, CONTROL_HOST_TO_DEVICE);
        rtos_printf("I2C write completed - device control status %d\n", ret);
    }
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t i2c_dev_ctrl_tx_start_cb(rtos_i2c_slave_t *ctx,
                                device_control_t *device_control_ctx,
                                uint8_t **data)
{
    control_ret_t ret;
    size_t len = RTOS_I2C_SLAVE_BUF_LEN;

    ret = device_control_payload_transfer(device_control_ctx,
                                          *data, &len, CONTROL_DEVICE_TO_HOST);
    rtos_printf("I2C read started - device control status %d\n", ret);

    if (ret != CONTROL_SUCCESS) {
        (*data)[0] = (control_status_t) ret;
        len = 1;
    }

    return len;
}
