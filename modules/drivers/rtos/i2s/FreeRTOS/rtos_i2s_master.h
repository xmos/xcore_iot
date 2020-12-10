// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_I2S_MASTER_H_
#define RTOS_I2S_MASTER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include <xcore/clock.h>
#include <xcore/port.h>
#include "i2s.h"

#include "drivers/rtos/rpc/api/rtos_driver_rpc.h"

typedef struct rtos_i2s_master_struct rtos_i2s_master_t;
struct rtos_i2s_master_struct{
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_i2s_master_tx_fptr_grp")))
    size_t (*tx)(rtos_i2s_master_t *, int32_t *, size_t);

    unsigned mclk_bclk_ratio;
    i2s_mode_t mode;
    port_t p_dout[I2S_MAX_DATALINES];
    size_t num_out;
    port_t p_din[I2S_MAX_DATALINES];
    size_t num_in;
    port_t p_bclk;
    port_t p_lrclk;
    port_t p_mclk;
    xclock_t bclk;

    /* BEGIN RTOS SPECIFIC MEMBERS. */
    StreamBufferHandle_t audio_stream_buffer;
};

#include "drivers/rtos/i2s/api/rtos_i2s_master_rpc.h"

inline size_t rtos_i2s_master_tx(
        rtos_i2s_master_t *ctx,
        int32_t *i2s_sample_buf,
        size_t frame_count)
{
    return ctx->tx(ctx, i2s_sample_buf, frame_count);
}

inline int rtos_i2s_master_mclk_bclk_ratio(
        const unsigned audio_clock_frequency,
        const unsigned sample_rate)
{
    return audio_clock_frequency / (sample_rate * (8 * sizeof(int32_t)) * I2S_CHANS_PER_FRAME);
}

void rtos_i2s_master_start(
        rtos_i2s_master_t *i2s_master_ctx,
        unsigned mclk_bclk_ratio,
        i2s_mode_t mode,
        size_t buffer_size,
        unsigned priority);

void rtos_i2s_master_init(
        rtos_i2s_master_t *i2s_master_ctx,
        port_t p_dout[],
        size_t num_out,
        port_t p_din[],
        size_t num_in,
        port_t p_bclk,
        port_t p_lrclk,
        port_t p_mclk,
        xclock_t bclk);


#endif /* RTOS_I2S_MASTER_H_ */
