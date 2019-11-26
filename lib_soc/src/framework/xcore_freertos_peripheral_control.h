// Copyright (c) 2019, XMOS Ltd, All rights reserved

/**
 * This file contains functions used for communication over
 * a control channel between a software driver and a bitstream
 * peripheral.
 *
 * It enables a bitstream peripheral to implement functions that
 * can be called from software indirectly via a control channel.
 * Devices do not have to, but may, be on the same tile as the
 * software driver.
 *
 * Think XC interfaces.
 */

#ifndef XCORE_FREERTOS_PERIPHERAL_CONTROL_H_
#define XCORE_FREERTOS_PERIPHERAL_CONTROL_H_

#ifdef __XC__
extern "C" {
#endif //__XC__

/** Send a function code to a bitstream peripheral.
 *
 * This function is intended to be used by a software driver to
 * initiate a control function in a bitstream peripheral.
 *
 * Any function parameters can subsequently be sent with
 * xcore_freertos_periph_varlist_tx(). Any return values
 * may be received with xcore_freertos_periph_varlist_rx().
 *
 * \param[in] c    The control chanend for the peripheral.
 * \param[in] code The function code to send.
 */
void xcore_freertos_periph_function_code_tx(
        chanend c,
        uint32_t code);

/** Send a list of parameters to a bitstream peripheral.
 *
 * This function is intended to be used by a software driver to
 * send a variable length list of parameters to a bitstream peripheral.
 *
 * It is also used by bitstream peripherals to send return values to
 * the software drivers.
 *
 * \param[in] c        The control chanend for the peripheral.
 * \param[in] num_args The number of parameters to send.
 * \param[in] ...      A pair for each parameter being sent. The first
 *                     value is an int with the parameter length. The
 *                     second is a pointer to the parameter to send.
 */
void xcore_freertos_periph_varlist_tx(
        chanend c,
        int num_args,
        ...);

/** Receive a function code from a software driver.
 *
 * This function is intended to be used by a bitstream peripheral.
 * It may be used as a select case. It receives the function
 * code sent by the software driver.
 *
 * \param[in]  c    The control chanend for the peripheral.
 * \param[out] code The received function code.
 */
#ifdef __XC__
#pragma select handler
#endif //__XC__
void xcore_freertos_periph_function_code_rx(
        chanend c,
        uint32_t *code);

/** Receives a list of return values from a bitstream peripheral.
 *
 * This function is intended to be used by a software driver to
 * receive a variable length list of return values from a bitstream
 * peripheral.
 *
 * It is also used by bitstream peripherals to receive parameters from
 * the software drivers.
 *
 * \param[in] c            The control chanend for the peripheral.
 * \param[in] num_args     The number of return values to receive.
 * \param[in,out] ...      A pair for each return value being received.
 *                         The first value is an int with the return
 *                         value length. The second is a pointer to where
 *                         the return value should be received.
 */
void xcore_freertos_periph_varlist_rx(
        chanend c,
        int num_args,
        ...);

#ifdef __XC__
}
#endif //__XC__

#endif /* XCORE_FREERTOS_PERIPHERAL_CONTROL_H_ */
