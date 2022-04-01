// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_MIC_ARRAY_H_
#define RTOS_MIC_ARRAY_H_

/**
 * \addtogroup rtos_mic_array_driver rtos_mic_array_driver
 *
 * The public API for using the RTOS mic array driver.
 * @{
 */

#include <xcore/channel.h>
#include <xcore/clock.h>
#include <xcore/port.h>
#include "mic_array.h"
#include "mic_array_vanilla.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * Typedef for the RTOS mic array driver audio format
 */
typedef enum
{
    RTOS_MIC_ARRAY_CHANNEL_SAMPLE,
    RTOS_MIC_ARRAY_SAMPLE_CHANNEL,
    RTOS_MIC_ARRAY_FORMAT_COUNT
} rtos_mic_array_format_t;

/**
 * Typedef to the RTOS mic array driver instance struct.
 */
typedef struct rtos_mic_array_struct rtos_mic_array_t;

/**
 * Struct representing an RTOS mic array driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_mic_array_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
    size_t (*rx)(rtos_mic_array_t *, int32_t **sample_buf, size_t, unsigned);

    streaming_channel_t c_pdm_mic;
    rtos_mic_array_format_t format;

    rtos_osal_thread_t hil_thread;
    rtos_osal_semaphore_t recv_sem;
    int recv_blocked;
    struct {
        int32_t *buf;
        size_t buf_size;
        size_t write_index;
        size_t read_index;
        volatile size_t total_written;
        volatile size_t total_read;
        volatile size_t required_available_count;
    } recv_buffer;
};

#include "rtos_mic_array_rpc.h"

/**
 * \addtogroup rtos_mic_array_driver_core rtos_mic_array_driver_core
 *
 * The core functions for using an RTOS mic array driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Receives sample frames from the PDM mic array interface.
 *
 * This function will block until new frames are available.
 *
 * \param ctx            A pointer to the mic array driver instance to use.
 * \param sample_buf     A buffer to copy the received sample frames into.
 * \param frame_count    The number of frames to receive from the buffer.
 *                       This must be less than or equal to the size of the
 *                       buffer specified to rtos_mic_array_start() if in
 *                       RTOS_MIC_ARRAY_SAMPLE_CHANNEL mode.  This must be equal
 *                       to MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME if in
 *                       RTOS_MIC_ARRAY_CHANNEL_SAMPLE mode.
 * \param timeout        The amount of time to wait before the requested number
 *                       of frames becomes available.
 *
 * \returns              The number of frames actually received into \p sample_buf.
 */
inline size_t rtos_mic_array_rx(
        rtos_mic_array_t *ctx,
        int32_t **sample_buf,
        size_t frame_count,
        unsigned timeout)
{
    return ctx->rx(ctx, sample_buf, frame_count, timeout);
}

/**@}*/

/**
 * Starts an RTOS mic array driver instance. This must only be called by the tile that
 * owns the driver instance. It must be called after starting the RTOS from an RTOS thread,
 * and must be called before any of the core mic array driver functions are called with this
 * instance.
 *
 * rtos_mic_array_init() must be called on this mic array driver instance prior to calling this.
 *
 * \param mic_array_ctx         A pointer to the mic array driver instance to start.
 * \param buffer_size           The size in frames of the input buffer. Each frame is two samples
 *                              (one for each microphone) plus one sample per reference channel.
 *                              This must be at least MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME. Samples are pulled out
 *                              of this buffer by the application by calling rtos_mic_array_rx().
 * \param interrupt_core_id     The ID of the core on which to enable the mic array interrupt.
 */
void rtos_mic_array_start(
        rtos_mic_array_t *mic_array_ctx,
        size_t buffer_size,
        unsigned interrupt_core_id);

/**
 * Initializes an RTOS mic array driver instance.
 * This must only be called by the tile that owns the driver instance. It should be
 * called before starting the RTOS, and must be called before calling rtos_mic_array_start()
 * or any of the core mic array driver functions with this instance.
 *
 * \param mic_array_ctx     A pointer to the mic array driver instance to initialize.
 * \param io_core_mask      A bitmask representing the cores on which the low level mic array
 *                          I/O thread created by the driver is allowed to run. Bit 0 is core 0,
 *                          bit 1 is core 1, etc.
 * \param format            Format of the output data
 */
void rtos_mic_array_init(
        rtos_mic_array_t *mic_array_ctx,
        uint32_t io_core_mask,
        rtos_mic_array_format_t format);

/**@}*/

#endif /* RTOS_MIC_ARRAY_H_ */
