// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_I2C_MASTER_H_
#define RTOS_I2C_MASTER_H_

#include "FreeRTOS.h"
#include "semphr.h"

#include "i2c.h"

#include "drivers/rtos/rpc/api/rtos_driver_rpc.h"

typedef struct rtos_i2c_master_struct rtos_i2c_master_t;
struct rtos_i2c_master_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_i2c_master_write_fptr_grp")))
    i2c_res_t (*write)(rtos_i2c_master_t *, uint8_t, uint8_t buf[], size_t, size_t *, int);

    __attribute__((fptrgroup("rtos_i2c_master_read_fptr_grp")))
    i2c_res_t (*read)(rtos_i2c_master_t *, uint8_t, uint8_t buf[], size_t, int);

    __attribute__((fptrgroup("rtos_i2c_master_stop_bit_send_fptr_grp")))
    void (*stop_bit_send)(rtos_i2c_master_t *);

    __attribute__((fptrgroup("rtos_i2c_master_reg_write_fptr_grp")))
    i2c_regop_res_t (*reg_write)(rtos_i2c_master_t *, uint8_t, uint8_t, uint8_t);

    __attribute__((fptrgroup("rtos_i2c_master_reg_read_fptr_grp")))
    i2c_regop_res_t (*reg_read)(rtos_i2c_master_t *, uint8_t, uint8_t, uint8_t *);

    i2c_master_t ctx;

    /* BEGIN RTOS SPECIFIC MEMBERS. */
    SemaphoreHandle_t lock;
};

#include "drivers/rtos/i2c/api/rtos_i2c_master_rpc.h"

inline i2c_res_t rtos_i2c_master_write(
        rtos_i2c_master_t *ctx,
        uint8_t device,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    return ctx->write(ctx, device, buf, n, num_bytes_sent, send_stop_bit);
}

inline i2c_res_t rtos_i2c_master_read(
        rtos_i2c_master_t *ctx,
        uint8_t device,
        uint8_t buf[],
        size_t m,
        int send_stop_bit)
{
    return ctx->read(ctx, device, buf, m, send_stop_bit);
}

inline void rtos_i2c_master_stop_bit_send(
        rtos_i2c_master_t *ctx)
{
    ctx->stop_bit_send(ctx);
}

inline i2c_regop_res_t  rtos_i2c_master_reg_write(
        rtos_i2c_master_t *ctx,
        uint8_t device,
        uint8_t reg,
        uint8_t data)
{
    return ctx->reg_write(ctx, device, reg, data);
}

inline i2c_regop_res_t  rtos_i2c_master_reg_read(
        rtos_i2c_master_t *ctx,
        uint8_t device,
        uint8_t reg,
        uint8_t *data)
{
    return ctx->reg_read(ctx, device, reg, data);
}

void rtos_i2c_master_start(
        rtos_i2c_master_t *i2c_master_ctx);

void rtos_i2c_master_init(
        rtos_i2c_master_t *i2c_master_ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        hwtimer_t tmr,
        const unsigned kbits_per_second);

#endif /* RTOS_I2C_MASTER_H_ */
