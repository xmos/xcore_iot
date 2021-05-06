// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_MIC_ARRAY_H_
#define RTOS_MIC_ARRAY_H_

/**
 * \defgroup rtos_mic_array_driver
 *
 * The public API for using the RTOS mic array driver.
 * @{
 */

#include <xcore/channel.h>
#include <xcore/clock.h>
#include <xcore/port.h>
#include "mic_array.h"

#include "rtos/osal/api/rtos_osal.h"
#include "rtos/drivers/rpc/api/rtos_driver_rpc.h"

/**
 * The number of microphones in the mic array.
 * Currently this is always 2 because the driver
 * only supports a single instance of the "dual"
 * mic array task.
 */
#define MIC_DUAL_NUM_CHANNELS 2

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

  __attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp"))) size_t (*rx)(
      rtos_mic_array_t *,
      int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
      size_t, unsigned);

  port_t p_pdm_mics;
  streaming_channel_t c_2x_pdm_mic;
#if MIC_DUAL_NUM_REF_CHANNELS > 0
  streaming_channel_t c_ref_audio[MIC_DUAL_NUM_REF_CHANNELS];
#endif

  int decimation_factor;
  mic_dual_third_stage_coef_t *third_stage_coefs;
  int fir_gain_compensation;

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

#include "rtos/drivers/mic_array/api/rtos_mic_array_rpc.h"

/**
 * \defgroup rtos_mic_array_driver_core
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
 *                       buffer specified to rtos_mic_array_start().
 * \param timeout        The amount of time to wait before the requested number
 *                       of frames becomes available.
 *
 * \returns              The number of frames actually received into \p
 * sample_buf.
 */
inline size_t rtos_mic_array_rx(
    rtos_mic_array_t *ctx,
    int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
    size_t frame_count, unsigned timeout) {
  return ctx->rx(ctx, sample_buf, frame_count, timeout);
}

/**@}*/

/**
 * Helper function to determine the mic array decimator's third stage
 * coefficients given the decimation factor.
 *
 * \note If the value for \p decimation_factor is known at compile time, then
 * much memory will be saved because the compiler will optimize out the unused
 * coefficient arrays since this is an inline function.
 *
 * \param decimation_factor The mic array decimation factor as calculated by
 *                          rtos_mic_array_decimation_factor().
 *
 * \returns the third stage coefficients that should be provided to
 * rtos_mic_array_start().
 */
inline mic_dual_third_stage_coef_t *rtos_mic_array_third_stage_coefs(
    const unsigned decimation_factor) {
  mic_dual_third_stage_coef_t *const fir_coefs[7] = {
      NULL,
      g_third_stage_div_2_fir_dual,
      g_third_stage_div_4_fir_dual,
      g_third_stage_div_6_fir_dual,
      g_third_stage_div_8_fir_dual,
      NULL,
      g_third_stage_div_12_fir_dual};

  return fir_coefs[decimation_factor / 2];
}

/**
 * Helper function to determine the mic array decimator's FIR compensation
 * value given the decimation factor.
 *
 * \param decimation_factor The mic array decimation factor as calculated by
 *                          rtos_mic_array_decimation_factor().
 *
 * \returns the FIR compensation value that should be provided to
 * rtos_mic_array_start().
 */
inline int rtos_mic_array_fir_compensation(const unsigned decimation_factor) {
  const int fir_gain_compen[7] = {0,
                                  FIR_COMPENSATOR_DIV_2,
                                  FIR_COMPENSATOR_DIV_4,
                                  FIR_COMPENSATOR_DIV_6,
                                  FIR_COMPENSATOR_DIV_8,
                                  0,
                                  FIR_COMPENSATOR_DIV_12};

  return fir_gain_compen[decimation_factor / 2];
}

/**
 * Helper function to calculate the mic array decimation factor given the
 * PDM mic clock frequency and the desired sample rate.
 *
 * \note If the values for \p pdm_clock_frequency and \p sample_rate are known
 * at compile time, then this function will be able to be computed at known at
 * compile time since it is an inline function. This is useful when passing the
 * returned value to rtos_mic_array_third_stage_coefs().
 *
 * \param pdm_clock_frequency The frequency of the PDM mic clock at the port
 * p_pdm_clk. \param sample_rate         The desired sample rate.
 *
 * \returns the decimation_factor that should be provided to
 * rtos_mic_array_start().
 */
inline int rtos_mic_array_decimation_factor(const unsigned pdm_clock_frequency,
                                            const unsigned sample_rate) {
  return pdm_clock_frequency / (8 * sizeof(int32_t)) / sample_rate;
}

/**
 * Starts an RTOS mic array driver instance. This must only be called by the
 * tile that owns the driver instance. It must be called after starting the RTOS
 * from an RTOS thread, and must be called before any of the core mic array
 * driver functions are called with this instance.
 *
 * rtos_mic_array_init() must be called on this mic array driver instance prior
 * to calling this.
 *
 * \param mic_array_ctx         A pointer to the mic array driver instance to
 * start. \param decimation_factor     The final stage FIR Decimation factor.
 * This may be determined by the helper function
 * rtos_mic_array_decimation_factor(). \param third_stage_coefs     The
 * coefficients for the final stage FIR decimator. This may be determined by the
 * helper function rtos_mic_array_third_stage_coefs(). \param
 * fir_gain_compensation The gain compensation for the FIR filter. This may be
 * determined by the helper function rtos_mic_array_fir_compensation(). \param
 * buffer_size           The size in frames of the input buffer. Each frame is
 * two samples (one for each microphone) plus one sample per reference channel.
 *                              This must be at least MIC_DUAL_FRAME_SIZE.
 * Samples are pulled out of this buffer by the application by calling
 * rtos_mic_array_rx(). \param interrupt_core_id     The ID of the core on which
 * to enable the mic array interrupt.
 */
void rtos_mic_array_start(rtos_mic_array_t *mic_array_ctx,
                          int decimation_factor,
                          mic_dual_third_stage_coef_t *third_stage_coefs,
                          int fir_gain_compensation, size_t buffer_size,
                          unsigned interrupt_core_id);

/**
 * Initializes an RTOS mic array driver instance.
 * This must only be called by the tile that owns the driver instance. It should
 * be called before starting the RTOS, and must be called before calling
 * rtos_mic_array_start() or any of the core mic array driver functions with
 * this instance.
 *
 * \param mic_array_ctx     A pointer to the mic array driver instance to
 * initialize. \param io_core_mask      A bitmask representing the cores on
 * which the low level mic array I/O thread created by the driver is allowed to
 * run. Bit 0 is core 0, bit 1 is core 1, etc. \param pdmclk            A clock
 * that will be configured to drive \p p_pdm_clk. \param pdmclk2           A
 * clock that must be specified if there are two mics in DDR mode per pin. It
 * will be configured and used to sample \p p_pdm_mics. \note The driver
 * currently only supports two microhones in DDR mode, so this must be
 * specified. \param pdm_clock_divider This should be the frequency of the audio
 * clock at the port p_mclk divided by the desired PDM clock frequency at \p
 * p_pdm_clk. \param p_mclk            Input port which supplies the master
 * clock. \param p_pdm_clk         1-bit output port which drives the
 * microphones' clock pins. \param p_pdm_mics        1-bit input port connected
 * to the microphones' data out pins.
 */
void rtos_mic_array_init(rtos_mic_array_t *mic_array_ctx, uint32_t io_core_mask,
                         const xclock_t pdmclk, const xclock_t pdmclk2,
                         const unsigned pdm_clock_divider, const port_t p_mclk,
                         const port_t p_pdm_clk, const port_t p_pdm_mics);

/**@}*/

#endif /* RTOS_MIC_ARRAY_H_ */
