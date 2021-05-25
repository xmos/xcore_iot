// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef I2C_TEST_H_
#define I2C_TEST_H_

#include "test_printf.h"

#define i2c_printf( FMT, ... )       module_printf("I2C", FMT, ##__VA_ARGS__)

void start_i2c_devices(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx);
int run_i2c_tests(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx);
void i2c_slave_test_stage_increment(void);

int master_reg_write_test_master(rtos_i2c_master_t *i2c_master_ctx);
int master_reg_write_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);

int master_reg_read_test_master(rtos_i2c_master_t *i2c_master_ctx);
int master_reg_read_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);
size_t master_reg_read_test_i2c_slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data);

int master_write_test_master(rtos_i2c_master_t *i2c_master_ctx);
int master_write_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);

int master_write_multiple_test_master(rtos_i2c_master_t *i2c_master_ctx);
int master_write_multiple_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);

#endif /* I2C_TEST_H_ */
