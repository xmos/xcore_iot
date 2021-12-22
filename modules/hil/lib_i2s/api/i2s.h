// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _i2s_h_
#define _i2s_h_
#include <xs1.h>
#include <stdint.h>
#include <stddef.h>
#include <xcore/port.h>
#include <xcore/clock.h>
#include <xcore/parallel.h>

/**
 * \addtogroup hil_i2s_core hil_i2s_core
 *
 * The public API for using the HIL I2S core.
 * @{
 */

#define I2S_MAX_DATALINES 8
#define I2S_CHANS_PER_FRAME 2

/**
 * I2S mode.
 *
 * This type is used to describe the I2S mode.
 */
typedef enum i2s_mode {
    I2S_MODE_I2S,            /**< The LR clock transitions ahead of the data by one bit clock. */
    I2S_MODE_LEFT_JUSTIFIED, /**< The LR clock and data are phase aligned. */
} i2s_mode_t;

/**
 * I2S slave bit clock polarity.
 *
 * Standard I2S is positive, that is toggle data and LR clock on falling
 * edge of bit clock and sample them on rising edge of bit clock. Some
 * masters have it the other way around.
 */
typedef enum i2s_slave_bclk_polarity {
    I2S_SLAVE_SAMPLE_ON_BCLK_RISING,   /**< Toggle falling, sample rising (default if not set) */
    I2S_SLAVE_SAMPLE_ON_BCLK_FALLING,  /**< Toggle rising, sample falling */
} i2s_slave_bclk_polarity_t;

/**
 * I2S configuration structure.
 *
 * This structure describes the configuration of an I2S bus.
 */
typedef struct i2s_config {
  unsigned mclk_bclk_ratio;                       /**< The ratio between the master clock and bit clock signals. */
  i2s_mode_t mode;                                /**< The mode of the LR clock. */
  i2s_slave_bclk_polarity_t slave_bclk_polarity;  /**< Slave bit clock polarity. */
} i2s_config_t;

/**
 * Restart command type.
 *
 * Restart commands that can be signalled to the I2S or TDM component.
 */
typedef enum i2s_restart {
  I2S_NO_RESTART = 0,      /**< Do not restart. */
  I2S_RESTART,             /**< Restart the bus (causes the I2S/TDM to stop and a new init callback to occur allowing reconfiguration of the BUS). */
  I2S_SHUTDOWN             /**< Shutdown. This will cause the I2S/TDM component to exit. */
} i2s_restart_t;

/**
 * I2S initialization event callback.
 *
 * The I2S component will call this
 * when it first initializes on first run of after a restart.
 *
 * \param app_data    Points to application specific data supplied
 *                    by the application. May be used for context
 *                    data specific to each I2S task instance.
 *
 * \param i2s_config  This structure is provided if the connected
 *                    component drives an I2S bus. The members
 *                    of the structure should be set to the
 *                    required configuration.
 */
typedef void (*i2s_init_t)(void *app_data, i2s_config_t *i2s_config);

/**
 * I2S restart check callback.
 *
 * This callback is called once per frame. The application must return the
 * required restart behavior.
 *
 * \param app_data  Points to application specific data supplied
 *                  by the application. May be used for context
 *                  data specific to each I2S task instance.
 *
 * \return          The return value should be set to
 *                  #I2S_NO_RESTART, #I2S_RESTART or
 *                  #I2S_SHUTDOWN.
 */
typedef i2s_restart_t (*i2s_restart_check_t)(void *app_data);

/**
 * Receive an incoming frame of samples.
 *
 * This callback will be called when a new frame of samples is read in by the I2S
 * task.
 *
 * \param app_data  Points to application specific data supplied
 *                  by the application. May be used for context
 *                  data specific to each I2S task instance.
 *
 * \param num_in    The number of input channels contained within the array.
 *
 * \param samples   The samples data array as signed 32-bit values.  The component
 *                  may not have 32-bits of accuracy (for example, many
 *                  I2S codecs are 24-bit), in which case the bottom bits
 *                  will be arbitrary values.
 */
typedef void (*i2s_receive_t)(void *app_data, size_t num_in, const int32_t *samples);

/**
 * Request an outgoing frame of samples.
 *
 * This callback will be called when the I2S task needs a new frame of samples.
 *
 * \param app_data  Points to application specific data supplied
 *                  by the application. May be used for context
 *                  data specific to each I2S task instance.
 *
 * \param num_out   The number of output channels contained within the array.
 *
 * \param samples   The samples data array as signed 32-bit values.  The component
 *                  may not have 32-bits of accuracy (for example, many
 *                  I2S codecs are 24-bit), in which case the bottom bits
 *                  will be arbitrary values.
 */
typedef void (*i2s_send_t)(void *app_data, size_t num_out, int32_t *samples);

/**
 * This attribute must be specified on all I2S callback functions
 * provided by the application.
 */
#define I2S_CALLBACK_ATTR __attribute__((fptrgroup("i2s_callback")))

/**
 * Callback group representing callback events that can occur during the
 * operation of the I2S task. Must be initialized by the application prior
 * to passing it to one of the I2S tasks.
 */
typedef struct {
    /** Pointer to the application's i2s_init_t function to be called by the I2S device */
    I2S_CALLBACK_ATTR i2s_init_t init;

    /** Pointer to the application's i2s_restart_check_t function to be called by the I2S device */
    I2S_CALLBACK_ATTR i2s_restart_check_t restart_check;

    /** Pointer to the application's i2s_receive_t function to be called by the I2S device */
    I2S_CALLBACK_ATTR i2s_receive_t receive;

    /** Pointer to the application's i2s_send_t function to be called by the I2S device */
    I2S_CALLBACK_ATTR i2s_send_t send;

    /** Pointer to application specific data which is passed to each callback. */
    void *app_data;
} i2s_callback_group_t;

/**@}*/ // END: addtogroup hil_i2s_core

DECLARE_JOB(i2s_master, (const i2s_callback_group_t *, const port_t *, const size_t, const port_t *, const size_t, const port_t, const port_t, const port_t, const xclock_t));
DECLARE_JOB(i2s_master_external_clock, (const i2s_callback_group_t *, const port_t *, const size_t, const port_t *, const size_t, const port_t, const port_t, const xclock_t));
DECLARE_JOB(i2s_slave, (const i2s_callback_group_t *, port_t *, const size_t, port_t *, const size_t, port_t, port_t, xclock_t));

/**
 * \addtogroup hil_i2s_master hil_i2s_master
 *
 * The public API for using the HIL I2S master.
 * @{
 */

/**
 * I2S master task
 *
 * This task performs I2S on the provided pins. It will perform callbacks over
 * the i2s_callback_group_t callback group to get/receive frames of data from the
 * application using this component.
 *
 * The task performs I2S master so will drive the word clock and
 * bit clock lines.
 *
 * \param i2s_cbg        The I2S callback group pointing to the application's
 *                       functions to use for initialization and getting and receiving
 *                       frames. Also points to application specific data which will
 *                       be shared between the callbacks.
 * \param p_dout         An array of data output ports
 * \param num_out        The number of output data ports
 * \param p_din          An array of data input ports
 * \param num_in         The number of input data ports
 * \param p_bclk         The bit clock output port
 * \param p_lrclk        The word clock output port
 * \param p_mclk         Input port which supplies the master clock
 * \param bclk           A clock that will get configured for use with
 *                       the bit clock
 */
void i2s_master(
        const i2s_callback_group_t *const i2s_cbg,
        const port_t p_dout[],
        const size_t num_out,
        const port_t p_din[],
        const size_t num_in,
        const port_t p_bclk,
        const port_t p_lrclk,
        const port_t p_mclk,
        const xclock_t bclk);

/**
 * I2S master task
 *
 * This task differs from i2s_master() in that \p bclk must already be configured to
 * the BCLK frequency. Other than that, it is identical.
 *
 * This task performs I2S on the provided pins. It will perform callbacks over
 * the i2s_callback_group_t callback group to get/receive frames of data from the
 * application using this component.
 *
 * The task performs I2S master so will drive the word clock and
 * bit clock lines.
 *
 * \param i2s_cbg        The I2S callback group pointing to the application's
 *                       functions to use for initialization and getting and receiving
 *                       frames. Also points to application specific data which will
 *                       be shared between the callbacks.
 * \param p_dout         An array of data output ports
 * \param num_out        The number of output data ports
 * \param p_din          An array of data input ports
 * \param num_in         The number of input data ports
 * \param p_bclk         The bit clock output port
 * \param p_lrclk        The word clock output port
 * \param bclk           A clock that is configured externally to be used as the bit clock
 */
void i2s_master_external_clock(
        const i2s_callback_group_t *const i2s_cbg,
        const port_t p_dout[],
        const size_t num_out,
        const port_t p_din[],
        const size_t num_in,
        const port_t p_bclk,
        const port_t p_lrclk,
        const xclock_t bclk);

/**@}*/ // END: addtogroup hil_i2s_master

/**
 * \addtogroup hil_i2s_slave hil_i2s_slave
 *
 * The public API for using the HIL I2S slave.
 * @{
 */

/**
 * I2S slave task
 *
 * This task performs I2S on the provided pins. It will perform callbacks over
 * the i2s_callback_group_t callback group to get/receive data from the application
 * using this component.
 *
 * The component performs I2S slave so will expect the word clock and
 * bit clock to be driven externally.
 *
 * \param i2s_cbg        The I2S callback group pointing to the application's
 *                       functions to use for initialization and getting and receiving
 *                       frames. Also points to application specific data which will
 *                       be shared between the callbacks.
 * \param p_dout         An array of data output ports
 * \param num_out        The number of output data ports
 * \param p_din          An array of data input ports
 * \param num_in         The number of input data ports
 * \param p_bclk         The bit clock input port
 * \param p_lrclk        The word clock input port
 * \param bclk           A clock that will get configured for use with
 *                       the bit clock
 */
void i2s_slave(
        const i2s_callback_group_t *const i2s_cbg,
        port_t p_dout[],
        const size_t num_out,
        port_t p_din[],
        const size_t num_in,
        port_t p_bclk,
        port_t p_lrclk,
        xclock_t bclk);

/**@}*/ // END: addtogroup hil_i2s_slave

#endif // _i2s_h_
