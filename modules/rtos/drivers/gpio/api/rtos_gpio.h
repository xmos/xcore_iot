// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_GPIO_H_
#define RTOS_GPIO_H_

/**
 * \addtogroup rtos_gpio_driver rtos_gpio_driver
 *
 * The public API for using the RTOS GPIO driver.
 * @{
 */

#include <xcore/port.h>

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * Enumerator type representing each available GPIO port.
 *
 * To be used with the RTOS GPIO driver functions.
 */
typedef enum {
    rtos_gpio_port_none = -1,
    rtos_gpio_port_1A,
    rtos_gpio_port_1B,
    rtos_gpio_port_1C,
    rtos_gpio_port_1D,
    rtos_gpio_port_1E,
    rtos_gpio_port_1F,
    rtos_gpio_port_1G,
    rtos_gpio_port_1H,
    rtos_gpio_port_1I,
    rtos_gpio_port_1J,
    rtos_gpio_port_1K,
    rtos_gpio_port_1L,
    rtos_gpio_port_1M,
    rtos_gpio_port_1N,
    rtos_gpio_port_1O,
    rtos_gpio_port_1P,
    rtos_gpio_port_4A,
    rtos_gpio_port_4B,
    rtos_gpio_port_4C,
    rtos_gpio_port_4D,
    rtos_gpio_port_4E,
    rtos_gpio_port_4F,
    rtos_gpio_port_8A,
    rtos_gpio_port_8B,
    rtos_gpio_port_8C,
    rtos_gpio_port_8D,
    rtos_gpio_port_16A,
    rtos_gpio_port_16B,
    rtos_gpio_port_16C,
    rtos_gpio_port_16D,
    rtos_gpio_port_32A,
    rtos_gpio_port_32B,
    RTOS_GPIO_TOTAL_PORT_CNT /**< Total number of I/O ports */
} rtos_gpio_port_id_t;

/**
 * This attribute must be specified on all RTOS GPIO interrupt callback functions
 * provided by the application.
 */
#define RTOS_GPIO_ISR_CALLBACK_ATTR __attribute__((fptrgroup("rtos_gpio_isr_cb_fptr_grp")))

/**
 * Typedef to the RTOS GPIO driver instance struct.
 */
typedef struct rtos_gpio_struct rtos_gpio_t;

/**
 * Function pointer type for application provided RTOS GPIO interrupt callback functions.
 *
 * These callback functions are called when there is a GPIO port interrupt.
 *
 * \param ctx           A pointer to the associated GPIO driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param port_id       The GPIO port that triggered the interrupt.
 * \param value         The value on the GPIO port that caused the interrupt.
 *                      \note this is the latched value that triggered the interrupt,
 *                      not the current value.
 */
typedef void (*rtos_gpio_isr_cb_t)(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value);

/**
 * Struct to hold interrupt state data for GPIO ports.
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct {
    RTOS_GPIO_ISR_CALLBACK_ATTR rtos_gpio_isr_cb_t callback;
    void *isr_app_data;
    int enabled;
    rtos_gpio_port_id_t port_id;
    rtos_gpio_t *ctx;
} rtos_gpio_isr_info_t;

/**
 * Struct representing an RTOS GPIO driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_gpio_struct {
    rtos_driver_rpc_t *rpc_config;
    chanend_t rpc_interrupt_c[RTOS_DRIVER_RPC_MAX_CLIENT_TILES];

    __attribute__((fptrgroup("rtos_gpio_port_enable_fptr_grp")))
    void (*port_enable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_port_in_fptr_grp")))
    uint32_t (*port_in)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_port_out_fptr_grp")))
    void (*port_out)(rtos_gpio_t *, rtos_gpio_port_id_t, uint32_t);

    __attribute__((fptrgroup("rtos_gpio_port_write_control_word_fptr_grp")))
    void (*port_write_control_word)(rtos_gpio_t *, rtos_gpio_port_id_t, uint32_t);

    __attribute__((fptrgroup("rtos_gpio_isr_callback_set_fptr_grp")))
    void (*isr_callback_set)(rtos_gpio_t *, rtos_gpio_port_id_t, rtos_gpio_isr_cb_t, void *);

    __attribute__((fptrgroup("rtos_gpio_interrupt_enable_fptr_grp")))
    void (*interrupt_enable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_interrupt_disable_fptr_grp")))
    void (*interrupt_disable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    rtos_gpio_isr_info_t *isr_info[RTOS_GPIO_TOTAL_PORT_CNT];

    rtos_osal_mutex_t lock; /* Only used by RPC client */
};

#include "rtos_gpio_rpc.h"

/**
 * Helper function to convert an xcore I/O port resource ID
 * to an RTOS GPIO driver port ID.
 *
 * \param p An xcore I/O port resource ID.
 *
 * \returns the equivalent RTOS GPIO driver port ID.
 */
inline rtos_gpio_port_id_t rtos_gpio_port(port_t p)
{
    switch (p) {
    case XS1_PORT_1A:
        return rtos_gpio_port_1A;
    case XS1_PORT_1B:
        return rtos_gpio_port_1B;
    case XS1_PORT_1C:
        return rtos_gpio_port_1C;
    case XS1_PORT_1D:
        return rtos_gpio_port_1D;
    case XS1_PORT_1E:
        return rtos_gpio_port_1E;
    case XS1_PORT_1F:
        return rtos_gpio_port_1F;
    case XS1_PORT_1G:
        return rtos_gpio_port_1G;
    case XS1_PORT_1H:
        return rtos_gpio_port_1H;
    case XS1_PORT_1I:
        return rtos_gpio_port_1I;
    case XS1_PORT_1J:
        return rtos_gpio_port_1J;
    case XS1_PORT_1K:
        return rtos_gpio_port_1K;
    case XS1_PORT_1L:
        return rtos_gpio_port_1L;
    case XS1_PORT_1M:
        return rtos_gpio_port_1M;
    case XS1_PORT_1N:
        return rtos_gpio_port_1N;
    case XS1_PORT_1O:
        return rtos_gpio_port_1O;
    case XS1_PORT_1P:
        return rtos_gpio_port_1P;
    case XS1_PORT_4A:
        return rtos_gpio_port_4A;
    case XS1_PORT_4B:
        return rtos_gpio_port_4B;
    case XS1_PORT_4C:
        return rtos_gpio_port_4C;
    case XS1_PORT_4D:
        return rtos_gpio_port_4D;
    case XS1_PORT_4E:
        return rtos_gpio_port_4E;
    case XS1_PORT_4F:
        return rtos_gpio_port_4F;
    case XS1_PORT_8A:
        return rtos_gpio_port_8A;
    case XS1_PORT_8B:
        return rtos_gpio_port_8B;
    case XS1_PORT_8C:
        return rtos_gpio_port_8C;
    case XS1_PORT_8D:
        return rtos_gpio_port_8D;
    case XS1_PORT_16A:
        return rtos_gpio_port_16A;
    case XS1_PORT_16B:
        return rtos_gpio_port_16B;
    case XS1_PORT_16C:
        return rtos_gpio_port_16C;
    case XS1_PORT_16D:
        return rtos_gpio_port_16D;
    case XS1_PORT_32A:
        return rtos_gpio_port_32A;
    case XS1_PORT_32B:
        return rtos_gpio_port_32B;
    default:
        return rtos_gpio_port_none;
    }
}

/**
 * \addtogroup rtos_gpio_driver_core rtos_gpio_driver_core
 *
 * The core functions for using an RTOS GPIO driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Enables a GPIO port. This must be called on a port before
 * using it with any other GPIO driver function.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  The GPIO port to enable.
 */
inline void rtos_gpio_port_enable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    ctx->port_enable(ctx, port_id);
}

/**
 * Inputs the value present on a GPIO port's pins.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  The GPIO port to read from.
 *
 * \returns the value on the port's pins.
 */
inline uint32_t rtos_gpio_port_in(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    return ctx->port_in(ctx, port_id);
}

/**
 * Outputs a value to a GPIO port's pins.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  The GPIO port to write to.
 * \param value    The value to write to the GPIO port.
 */
inline void rtos_gpio_port_out(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id,
        uint32_t value)
{
    ctx->port_out(ctx, port_id, value);
}

/**
 * Sets the application callback function to be called when there is an
 * interrupt on a GPIO port.
 *
 * This must be called prior to enabling interrupts on \p port_id.
 * It is also safe to be called while interrupts are enabled on it.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  Interrupts triggered by this port will call the application
 *                 callback function \p cb.
 * \param cb       The application callback function to call when there is an
 *                 interrupt triggered by the port \p port_id.
 * \param app_data A pointer to application specific data to pass to the application
 *                 callback function \p cb.
 */
inline void rtos_gpio_isr_callback_set(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id,
        rtos_gpio_isr_cb_t cb,
        void *app_data)
{
    ctx->isr_callback_set(ctx, port_id, cb, app_data);
}

/**
 * Enables interrupts on a GPIO port. Interrupts are triggered whenever
 * the value on the port changes.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  The GPIO port to enable interrupts on.
 */
inline void rtos_gpio_interrupt_enable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    ctx->interrupt_enable(ctx, port_id);
}

/**
 * Disables interrupts on a GPIO port.
 *
 * \param ctx      A pointer to the GPIO driver instance to use.
 * \param port_id  The GPIO port to disable interrupts on.
 */
inline void rtos_gpio_interrupt_disable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    ctx->interrupt_disable(ctx, port_id);
}

/**
* Configures a port in drive mode.  Output values will be driven
* on the pins.  This is the default drive state of a port.  This has
* the side effect of disabling the port's internal pull-up and
* pull down resistors.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to drive mode.
*/
inline void rtos_gpio_port_drive(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_DRIVE);
}

/**
* Configures a port in drive low mode.  When the output value is 0
* the pin is driven low, otherwise no value is driven.  This has
* the side effect of enabled the port's internal pull-up resistor.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to drive mode low.
*/
inline void rtos_gpio_port_drive_low(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_PULL_UP);
}

/**
* Configures a port in drive high mode.  When the output value is 1
* the pin is driven high, otherwise no value is driven.  This has
* the side effect of enabled the port's internal pull-down resistor.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to drive mode high.
*/
inline void rtos_gpio_port_drive_high(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_PULL_DOWN);
}

/**
* Disables the port's internal pull-up and pull down resistors.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to pull none mode.
*/
inline void rtos_gpio_port_pull_none(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_DRIVE);
}

/**
* Enables the port's internal pull-up resistor.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to pull up mode.
*/
inline void rtos_gpio_port_pull_up(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_PULL_UP);
}

/**
* Enables the port's internal pull-down resistor.
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to set to pull down mode.
*/
inline void rtos_gpio_port_pull_down(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id)
{
   return ctx->port_write_control_word(ctx, port_id, XS1_SETC_DRIVE_PULL_DOWN);
}

/**
* Configures the port control word value
*
* \param ctx      A pointer to the GPIO driver instance to use.
* \param port_id  The GPIO port to modify
* \param value    The value to set the control word to
*/
inline void rtos_gpio_write_control_word(
       rtos_gpio_t *ctx,
       rtos_gpio_port_id_t port_id,
       uint32_t value)
{
   return ctx->port_write_control_word(ctx, port_id, value);
}

/**@}*/

/**
 * Starts an RTOS GPIO driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core GPIO driver functions are
 * called with this instance.
 *
 * rtos_gpio_init() must be called on this GPIO driver instance prior to calling this.
 *
 * \param ctx A pointer to the GPIO driver instance to start.
 */
void rtos_gpio_start(
        rtos_gpio_t *ctx);

/**
 * Initializes an RTOS GPIO driver instance. There should only be one per tile.
 * This instance represents all the GPIO ports owned by the calling tile.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_gpio_start() or any of the core GPIO driver functions with this instance.
 *
 * \param ctx A pointer to the GPIO driver instance to initialize.
 */
void rtos_gpio_init(
        rtos_gpio_t *ctx);

/**@}*/

#endif /* RTOS_GPIO_H_ */
