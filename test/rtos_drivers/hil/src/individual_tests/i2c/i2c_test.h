// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef I2C_TEST_H_
#define I2C_TEST_H_

#include "rtos_test/rtos_test_utils.h"

#define i2c_printf( FMT, ... )       module_printf("I2C", FMT, ##__VA_ARGS__)

#define I2C_MAX_TESTS   12

#define I2C_MAIN_TEST_ATTR              __attribute__((fptrgroup("rtos_test_i2c_main_test_fptr_grp")))
#define I2C_SLAVE_RX_ATTR               __attribute__((fptrgroup("rtos_test_i2c_slave_rx_fptr_grp")))
#define I2C_SLAVE_TX_START_ATTR         __attribute__((fptrgroup("rtos_test_i2c_slave_tx_start_fptr_grp")))
#define I2C_SLAVE_TX_DONE_ATTR          __attribute__((fptrgroup("rtos_test_i2c_slave_tx_done_fptr_grp")))
#define I2C_SLAVE_RX_BYTE_CHECK_ATTR    __attribute__((fptrgroup("rtos_test_i2c_slave_rx_byte_check_fptr_grp")))
#define I2C_SLAVE_WRITE_ADDR_REQ_ATTR __attribute__((fptrgroup("rtos_test_i2c_slave_write_addr_check_fptr_grp")))

typedef struct i2c_test_ctx i2c_test_ctx_t;

struct i2c_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[I2C_MAX_TESTS];

    rtos_i2c_master_t *i2c_master_ctx;
    rtos_i2c_slave_t *i2c_slave_ctx;
    int slave_success[I2C_MAX_TESTS];

    I2C_MAIN_TEST_ATTR int (*main_test[I2C_MAX_TESTS])(i2c_test_ctx_t *ctx);
    I2C_SLAVE_RX_ATTR void (*slave_rx[I2C_MAX_TESTS])(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);
    I2C_SLAVE_TX_START_ATTR size_t (*slave_tx_start[I2C_MAX_TESTS])(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data);
    I2C_SLAVE_TX_DONE_ATTR void (*slave_tx_done[I2C_MAX_TESTS])(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);
    I2C_SLAVE_RX_BYTE_CHECK_ATTR void (*slave_rx_check_byte[I2C_MAX_TESTS])(rtos_i2c_slave_t *ctx, void *app_data, uint8_t data, i2c_slave_ack_t *cur_status);
    I2C_SLAVE_WRITE_ADDR_REQ_ATTR void (*slave_write_addr_req[I2C_MAX_TESTS])(rtos_i2c_slave_t *ctx, void *app_data, i2c_slave_ack_t *cur_status);
};

typedef int (*i2c_main_test_t)(i2c_test_ctx_t *ctx);
typedef void (*i2c_slave_rx_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);
typedef size_t (*i2c_slave_tx_start_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data);
typedef void (*i2c_slave_tx_done_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);
typedef void (*i2c_slave_rx_byte_check_cb_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t data, i2c_slave_ack_t *cur_status);
typedef void (*i2c_slave_write_addr_req_cb_t)(rtos_i2c_slave_t *ctx, void *app_data, i2c_slave_ack_t *cur_status);

int i2c_device_tests(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx, chanend_t c);

/* Local Tests */
void register_master_reg_write_test(i2c_test_ctx_t *test_ctx);
void register_master_reg_read_test(i2c_test_ctx_t *test_ctx);
void register_master_write_test(i2c_test_ctx_t *test_ctx);
void register_master_write_multiple_test(i2c_test_ctx_t *test_ctx);
void register_master_read_test(i2c_test_ctx_t *test_ctx);
void register_master_read_multiple_test(i2c_test_ctx_t *test_ctx);

/* RPC Tests */
void register_rpc_master_reg_write_test(i2c_test_ctx_t *test_ctx);
void register_rpc_master_reg_read_test(i2c_test_ctx_t *test_ctx);
void register_rpc_master_write_test(i2c_test_ctx_t *test_ctx);
void register_rpc_master_write_multiple_test(i2c_test_ctx_t *test_ctx);
void register_rpc_master_read_test(i2c_test_ctx_t *test_ctx);
void register_rpc_master_read_multiple_test(i2c_test_ctx_t *test_ctx);

#endif /* I2C_TEST_H_ */
