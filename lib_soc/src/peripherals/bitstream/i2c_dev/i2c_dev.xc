// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "xassert.h"

#include "i2c.h"
#include "i2c_dev.h"

[[combinable]]
void i2c_dev(
        chanend ctrl_c,
        client i2c_master_if i2c)
{
    uint32_t cmd;

    i2c_res_t res;
    i2c_regop_res_t reg_res;
    uint8_t device_addr;
    size_t n;
    int send_stop_bit;
    size_t num_bytes_sent;
    uint8_t reg;
    uint8_t data;

    uint8_t buf[I2CCONF_MAX_BUF_LEN];

    while (1) {

        select {
        case soc_peripheral_function_code_rx(ctrl_c, &cmd):

            switch (cmd) {

            case I2C_DEV_WRITE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 3,
                        sizeof(device_addr), &device_addr,
                        sizeof(n), &n,
                        sizeof(send_stop_bit), &send_stop_bit);

                xassert(n <= I2CCONF_MAX_BUF_LEN);

                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        n, buf);

                res = i2c.write(device_addr, buf, n, num_bytes_sent, send_stop_bit);

                soc_peripheral_varlist_tx(
                        ctrl_c, 2,
                        sizeof(num_bytes_sent), &num_bytes_sent,
                        sizeof(res), &res);
                break;

            case I2C_DEV_READ:
                soc_peripheral_varlist_rx(
                        ctrl_c, 3,
                        sizeof(device_addr), &device_addr,
                        sizeof(n), &n,
                        sizeof(send_stop_bit), &send_stop_bit);

                xassert(n <= I2CCONF_MAX_BUF_LEN);

                res = i2c.read(device_addr, buf, n, send_stop_bit);

                soc_peripheral_varlist_tx(
                        ctrl_c, 2,
                        n, buf,
                        sizeof(res), &res);

                break;

            case I2C_DEV_WRITE_REG:
                soc_peripheral_varlist_rx(
                        ctrl_c, 3,
                        sizeof(device_addr), &device_addr,
                        sizeof(reg), &reg,
                        sizeof(data), &data);

                reg_res = i2c.write_reg(device_addr, reg, data);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(reg_res), &reg_res);

                break;

            case I2C_DEV_READ_REG:
                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(device_addr), &device_addr,
                        sizeof(reg), &reg);

                data = i2c.read_reg(device_addr, reg, reg_res);

                soc_peripheral_varlist_tx(
                        ctrl_c, 2,
                        sizeof(reg_res), &reg_res,
                        sizeof(data), &data);

                break;

            default:
                /* I2C DEV RECEIVED INVALID CODE */ xassert(0);
                break;
            }

            break;
        }
    }
}
