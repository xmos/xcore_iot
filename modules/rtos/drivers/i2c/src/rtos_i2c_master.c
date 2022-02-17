// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_I2C

#include "rtos_i2c_master.h"

__attribute__((fptrgroup("rtos_i2c_master_write_fptr_grp")))
static i2c_res_t i2c_master_local_write(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    i2c_res_t res;

    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    res = i2c_master_write(
                           &ctx->ctx,
                           device_addr,
                           buf,
                           n,
                           num_bytes_sent,
                           send_stop_bit);

    rtos_osal_mutex_put(&ctx->lock);

    return res;
}

__attribute__((fptrgroup("rtos_i2c_master_read_fptr_grp")))
static i2c_res_t i2c_master_local_read(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    i2c_res_t res;

    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    res = i2c_master_read(
                          &ctx->ctx,
                          device_addr,
                          buf,
                          n,
                          send_stop_bit);

    rtos_osal_mutex_put(&ctx->lock);

    return res;
}

__attribute__((fptrgroup("rtos_i2c_master_stop_bit_send_fptr_grp")))
static void i2c_master_local_stop_bit_send(
        rtos_i2c_master_t *ctx)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    i2c_master_stop_bit_send(&ctx->ctx);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_i2c_master_reg_write_fptr_grp")))
static i2c_regop_res_t i2c_master_local_reg_write(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t data)
{
    i2c_regop_res_t reg_res;
    i2c_res_t res;
    size_t num_bytes_sent = 0;
    uint8_t buf[2];

    buf[0] = reg_addr;
    buf[1] = data;

    res = i2c_master_local_write(ctx, device_addr, buf, 2, &num_bytes_sent, 1);

    if (res == I2C_ACK) {
        if (num_bytes_sent == 0) {
            reg_res = I2C_REGOP_DEVICE_NACK;
        } else if (num_bytes_sent < 2) {
            reg_res = I2C_REGOP_INCOMPLETE;
        } else {
            reg_res = I2C_REGOP_SUCCESS;
        }
    } else {
        reg_res = I2C_REGOP_DEVICE_NACK;
    }

    return reg_res;
}

__attribute__((fptrgroup("rtos_i2c_master_reg_read_fptr_grp")))
static i2c_regop_res_t  i2c_master_local_reg_read(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t *data)
{
    i2c_regop_res_t reg_res;
    i2c_res_t res;
    size_t num_bytes_sent = 0;

    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    res = i2c_master_local_write(ctx, device_addr, &reg_addr, 1, &num_bytes_sent, 0);

    if (res == I2C_ACK) {
        if (num_bytes_sent == 0) {
            reg_res = I2C_REGOP_DEVICE_NACK;
        } else {
            reg_res = I2C_REGOP_SUCCESS;
        }
    } else {
        reg_res = I2C_REGOP_DEVICE_NACK;
    }

    if (reg_res == I2C_REGOP_SUCCESS) {
        res = i2c_master_local_read(ctx, device_addr, data, 1, 1);

        if (res == I2C_ACK) {
            reg_res = I2C_REGOP_SUCCESS;
        } else {
            reg_res = I2C_REGOP_DEVICE_NACK;
        }
    }

    rtos_osal_mutex_put(&ctx->lock);

    return reg_res;
}

void rtos_i2c_master_start(
        rtos_i2c_master_t *i2c_master_ctx)
{
    rtos_osal_mutex_create(&i2c_master_ctx->lock, "i2c_master_lock", RTOS_OSAL_RECURSIVE);

    if (i2c_master_ctx->rpc_config != NULL && i2c_master_ctx->rpc_config->rpc_host_start != NULL) {
        i2c_master_ctx->rpc_config->rpc_host_start(i2c_master_ctx->rpc_config);
    }
}

void rtos_i2c_master_init(
        rtos_i2c_master_t *i2c_master_ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        hwtimer_t tmr,
        const unsigned kbits_per_second)
{
    i2c_master_init(
            &i2c_master_ctx->ctx,
            p_scl, scl_bit_position, scl_other_bits_mask,
            p_sda, sda_bit_position, sda_other_bits_mask,
            kbits_per_second);

    xassert(tmr == 0);

    /*
     * TODO: Setting all of these here results in all these functions
     * getting linked into the binary, regardless of whether or not
     * they are used. Is there a clean way to prevent this?
     */
    i2c_master_ctx->rpc_config = NULL;
    i2c_master_ctx->write = i2c_master_local_write;
    i2c_master_ctx->read = i2c_master_local_read;
    i2c_master_ctx->stop_bit_send = i2c_master_local_stop_bit_send;
    i2c_master_ctx->reg_write = i2c_master_local_reg_write;
    i2c_master_ctx->reg_read = i2c_master_local_reg_read;
}
