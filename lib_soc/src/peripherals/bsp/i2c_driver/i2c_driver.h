// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

#include "soc.h"
#include "i2c_dev_ctrl.h"

typedef enum {
  I2C_NACK,    ///< the slave has NACKed the last byte
  I2C_ACK,     ///< the slave has ACKed the last byte
} i2c_res_t;

typedef enum {
  I2C_REGOP_SUCCESS,     ///< the operation was successful
  I2C_REGOP_DEVICE_NACK, ///< the operation was NACKed when sending the device address, so either the device is missing or busy
  I2C_REGOP_INCOMPLETE   ///< the operation was NACKed halfway through by the slave
} i2c_regop_res_t;

i2c_res_t i2c_driver_write(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit);

i2c_res_t i2c_driver_read(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit);

i2c_regop_res_t i2c_driver_write_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        uint8_t data);

uint8_t i2c_driver_read_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result);

soc_peripheral_t i2c_driver_init(
        int device_id);

#endif /* I2C_DRIVER_H_ */
