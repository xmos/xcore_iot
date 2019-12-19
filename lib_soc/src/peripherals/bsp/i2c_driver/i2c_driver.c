// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "i2c_driver.h"

#if ( SOC_I2C_PERIPHERAL_USED == 0 )
#define BITSTREAM_I2C_DEVICE_COUNT 0
soc_peripheral_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];
#endif /* SOC_I2C_PERIPHERAL_USED */

i2c_res_t i2c_driver_write(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    i2c_res_t res;
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, I2C_DEV_WRITE);

    soc_peripheral_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(n), &n,
            sizeof(send_stop_bit), &send_stop_bit);

    soc_peripheral_varlist_tx(
            c, 1,
            n, buf);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(size_t), num_bytes_sent,
            sizeof(res), &res);

    return res;
}

i2c_res_t i2c_driver_read(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    i2c_res_t res;
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, I2C_DEV_READ);

    soc_peripheral_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(n), &n,
            sizeof(send_stop_bit), &send_stop_bit);

    soc_peripheral_varlist_rx(
            c, 2,
            n, buf,
            sizeof(res), &res);

    return res;
}

i2c_regop_res_t i2c_driver_write_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        uint8_t data)
{
    i2c_regop_res_t res;
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, I2C_DEV_WRITE_REG);

    soc_peripheral_varlist_tx(
            c, 3,
            sizeof(device_addr), &device_addr,
            sizeof(reg), &reg,
            sizeof(data), &data);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(res), &res);

    return res;
}

uint8_t i2c_driver_read_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result)
{
    uint8_t data;
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, I2C_DEV_READ_REG);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(device_addr), &device_addr,
            sizeof(reg), &reg);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(i2c_regop_res_t), result,
            sizeof(data), &data);

    return data;
}

soc_peripheral_t i2c_driver_init(
        int device_id)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_I2C_DEVICE_COUNT);

    device = bitstream_i2c_devices[device_id];

    return device;
}
