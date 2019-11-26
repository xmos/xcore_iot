// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <stdlib.h>

#include "soc.h"
#include "bitstream_devices.h"

#include "i2c_driver.h"

i2c_res_t i2c_driver_write(
        xcore_freertos_device_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    i2c_res_t res;
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, I2C_DEV_WRITE);

    xcore_freertos_periph_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(n), &n,
            sizeof(send_stop_bit), &send_stop_bit);

    xcore_freertos_periph_varlist_tx(
            c, 1,
            n, buf);

    xcore_freertos_periph_varlist_rx(
            c, 2,
            sizeof(size_t), num_bytes_sent,
            sizeof(res), &res);

    return res;
}

i2c_res_t i2c_driver_read(
        xcore_freertos_device_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    i2c_res_t res;
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, I2C_DEV_READ);

    xcore_freertos_periph_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(n), &n,
            sizeof(send_stop_bit), &send_stop_bit);

    xcore_freertos_periph_varlist_rx(
            c, 2,
            n, buf,
            sizeof(res), &res);

    return res;
}

i2c_regop_res_t i2c_driver_write_reg(
        xcore_freertos_device_t dev,
        uint8_t device_addr,
        uint8_t reg,
        uint8_t data)
{
    i2c_regop_res_t res;
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, I2C_DEV_WRITE_REG);

    xcore_freertos_periph_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(reg), &reg,
            sizeof(data), &data);

    xcore_freertos_periph_varlist_rx(
            c, 1,
            sizeof(res), &res);

    return res;
}

uint8_t i2c_driver_read_reg(
        xcore_freertos_device_t dev,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result)
{
    uint8_t data;
    chanend c = xcore_freertos_dma_device_ctrl_chanend(dev);

    xcore_freertos_periph_function_code_tx(c, I2C_DEV_READ_REG);

    xcore_freertos_periph_varlist_tx(
            c, 2,
            sizeof(device_addr), &device_addr,
            sizeof(reg), &reg);

    xcore_freertos_periph_varlist_rx(
            c, 2,
            sizeof(i2c_regop_res_t), result,
            sizeof(data), &data);

    return data;
}

xcore_freertos_device_t i2c_driver_init(
        int device_id)
{
    xcore_freertos_device_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_I2C_DEVICE_COUNT);

    device = bitstream_i2c_devices[device_id];

    return device;
}
