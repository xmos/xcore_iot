// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

#ifndef RTOS_I2S_MASTER_H_
#define RTOS_I2S_MASTER_H_

/**
 * \defgroup rtos_i2s_master_driver
 *
 * The public API for using the RTOS I2S master driver.
 * @{
 */

#include <xcore/clock.h>
#include <xcore/port.h>
#include "i2s.h"

#include "rtos/drivers/osal/api/rtos_osal.h"
#include "rtos/drivers/rpc/api/rtos_driver_rpc.h"

/**
 * Typedef to the RTOS I2S master driver instance struct.
 */
typedef struct rtos_i2s_master_struct rtos_i2s_master_t;

/**
 * Struct representing an RTOS I2S master driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_i2s_master_struct{
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_i2s_master_tx_fptr_grp")))
    size_t (*tx)(rtos_i2s_master_t *, int32_t *, size_t, unsigned);

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

    streaming_channel_t c_i2s_isr;
    rtos_osal_semaphore_t send_sem;
    int send_blocked;
    struct {
        int32_t *buf;
        size_t buf_size;
        size_t write_index;
        size_t read_index;
        volatile size_t total_written;
        volatile size_t total_read;
        volatile size_t required_free_count;
    } send_buffer;
};

#include "rtos/drivers/i2s/api/rtos_i2s_master_rpc.h"

/**
 * \defgroup rtos_i2s_master_driver_core
 *
 * The core functions for using an RTOS I2S master driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Transmits sample frames out to the I2S interface.
 *
 * The samples are stored into a buffer and are not necessarily sent out to the
 * I2S interface before this function returns.
 *
 * \param ctx            A pointer to the I2S master driver instance to use.
 * \param i2s_sample_buf A buffer containing the sample frames to transmit out
 *                       to the I2S interface.
 * \param frame_count    The number of frames to transmit out from the buffer.
 *                       This must be less than or equal to the size of the
 *                       output buffer specified to rtos_i2s_master_start().
 * \param timeout        The amount of time to wait before there is enough
 *                       space in the send buffer to accept the frames to be
 *                       transmitted.
 *
 * \returns              The number of frames actually stored into the buffer.
 */
inline size_t rtos_i2s_master_tx(
        rtos_i2s_master_t *ctx,
        int32_t *i2s_sample_buf,
        size_t frame_count,
        unsigned timeout)
{
    return ctx->tx(ctx, i2s_sample_buf, frame_count, timeout);
}

/**@}*/

/**
 * Helper function to calculate the MCLK/BCLK ratio given the
 * audio clock frequency at the master clock pin and the
 * desired sample rate.
 *
 * \param audio_clock_frequency The frequency of the audio clock at the port p_mclk.
 * \param sample_rate           The desired sample rate.
 *
 * \returns the MCLK/BCLK ratio that should be provided to rtos_i2s_master_start().
 */
inline int rtos_i2s_master_mclk_bclk_ratio(
        const unsigned audio_clock_frequency,
        const unsigned sample_rate)
{
    return audio_clock_frequency / (sample_rate * (8 * sizeof(int32_t)) * I2S_CHANS_PER_FRAME);
}

/**
 * Starts an RTOS I2S master driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core I2S master driver functions are
 * called with this instance.
 *
 * rtos_i2s_master_init() must be called on this I2S master driver instance prior to calling this.
 *
 * \param i2s_master_ctx  A pointer to the I2S master driver instance to start.
 * \param mclk_bclk_ratio The master clock to bit clock ratio. This may be computed
 *                        by the helper function rtos_i2s_master_mclk_bclk_ratio().
 * \param mode            The mode of the LR clock. See i2s_mode_t.
 * \param buffer_size     The size in frames of the output buffer. Each frame is two samples
 *                        (left and right channels) per output port. For example, a size of two
 *                        here when num_out is three would create a buffer that holds up to
 *                        12 samples.
 *                        Frames transmitted by rtos_i2s_master_tx() are stored in this
 *                        buffers before they are sent out to the I2S interface.
 * \param priority        The priority of the task that gets created by the driver to
 *                        handle the I2S interface.
 */
void rtos_i2s_master_start(
        rtos_i2s_master_t *i2s_master_ctx,
        unsigned mclk_bclk_ratio,
        i2s_mode_t mode,
        size_t buffer_size,
        unsigned priority);

/**
 * Initializes the I2S master driver's interrupt and ISR. Must be called on the core
 * that will process the interrupts, prior to calling rtos_i2s_master_start().
 * It is recommended that the core the ISR runs on is not the same as the core that
 * runs the I2S master task started by rtos_i2s_master_start().
 *
 * \param i2s_master_ctx A pointer to the I2S master driver instance for which to initialize interrupts.
 */
void rtos_i2s_master_interrupt_init(rtos_i2s_master_t *i2s_master_ctx);

/**
 * Initializes an RTOS I2S master driver instance.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_i2s_master_start() or any of the core I2S master driver functions with this instance.
 *
 * \param i2s_master_ctx A pointer to the I2S master driver instance to initialize.
 * \param p_dout         An array of data output ports.
 * \param num_out        The number of output data ports.
 * \param p_din          An array of data input ports.
 * \param num_in         The number of input data ports.
 * \param p_bclk         The bit clock output port.
 * \param p_lrclk        The word clock output port.
 * \param p_mclk         Input port which supplies the master clock.
 * \param bclk           A clock that will get configured for use with
 *                       the bit clock.
 */
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

/**@}*/

#endif /* RTOS_I2S_MASTER_H_ */
