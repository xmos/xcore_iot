// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * Facilitates channel communication between tiles.
 * Essentially a thin wrapper around a streaming channel.
 *
 * Recommend limiting to one per tile pair. There should be at
 * least one more RTOS core usable by all tasks that use these
 * intertile links to handle the case where a transmit occurs
 * on both sides of all links at the same time. There must be
 * at least one core available to handle a receive or else
 * dead-lock may occur.
 */

#ifndef RTOS_INTERTILE_H_
#define RTOS_INTERTILE_H_

/**
 * \addtogroup rtos_intertile_driver rtos_intertile_driver
 *
 * The public API for using the RTOS intertile driver.
 * @{
 */

#include <xcore/channel.h>
#include <xcore/channel_transaction.h>

#include "rtos_osal.h"

/**
 * Struct representing an RTOS intertile driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct {
    chanend_t c;

    size_t tx_len;
    size_t rx_len;
    rtos_osal_mutex_t lock;
    rtos_osal_event_group_t event_group;
} rtos_intertile_t;

/**
 * Struct to hold an address to a remote function, consisting
 * of both an intertile instance and a port number. Primarily
 * used by the RPC mechanism in the RTOS drivers.
 */
typedef struct {
    rtos_intertile_t *intertile_ctx; /**< Intertile driver instance */
    int port;                        /**< Port number to the remote function */
} rtos_intertile_address_t;

/**
 * \addtogroup rtos_intertile_driver_core rtos_intertile_driver_core
 *
 * The core functions for using an RTOS intertile driver instance after
 * it has been initialized and started.
 * @{
 */

void rtos_intertile_tx_len(
        rtos_intertile_t *ctx,
        uint8_t port,
        size_t len);
size_t rtos_intertile_tx_data(
        rtos_intertile_t *ctx,
        void *data,
        size_t len);

/**
 * Transmits data to an intertile link.
 *
 * \param ctx  A pointer to the intertile driver instance to use.
 * \param port The number of the port to send the data to. Only the thread
 *             listening on this particular port on the remote tile will receive
 *             this data.
 * \param msg  A pointer to the data buffer to transmit.
 * \param len  The number of bytes from the buffer to transmit.
 */
void rtos_intertile_tx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void *msg,
        size_t len);

size_t rtos_intertile_rx_len(
        rtos_intertile_t *ctx,
        uint8_t port,
        unsigned timeout);
size_t rtos_intertile_rx_data(
        rtos_intertile_t *ctx,
        void *data,
        size_t len);

/**
 * Receives data from an intertile link.
 *
 * \note the buffer returned via \p msg must be freed by the
 * application using rtos_osal_free().
 *
 * \param ctx     A pointer to the intertile driver instance to use.
 * \param port    The number of the port to listen for data on. Only
 *                data sent to this port by the remote tile will be
 *                received.
 *                \note It is important that no other thread listen
 *                on this port simultaneously. If this happens, it
 *                is undefined which one will receive the data, and
 *                it is possible for a resource exception to occur.
 * \param msg     A pointer to the received data is written to this
 *                pointer variable. This buffer is obtained from the
 *                heap and must be freed by the application using
 *                rtos_osal_free().
 * \param timeout The amount of time to wait before data become
 *                available.
 *
 * \returns the number of bytes received.
 */
size_t rtos_intertile_rx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void **msg,
        unsigned timeout);

/**@}*/

/**
 * Starts an RTOS intertile driver instance. It may be called either before or after
 * starting the RTOS, but must be called before any of the core intertile driver functions
 * are called with this instance.
 *
 * rtos_intertile_init() must be called on this intertile driver instance prior to calling this.
 *
 * \param intertile_ctx A pointer to the intertile driver instance to start.
 */
void rtos_intertile_start(
        rtos_intertile_t *intertile_ctx);

/**
 * Initializes an RTOS intertile driver instance. This must be called simultaneously on
 * the two tiles establishing an intertile link. It may be called either before or after
 * starting the RTOS, but must be called before calling rtos_intertile_start() or any of
 * the core RTOS intertile functions with this instance.
 *
 * This establishes a new streaming channel between the two tiles, using the provided
 * non-streaming channel to bootstrap this.
 *
 * \param intertile_ctx A pointer to the intertile driver instance to initialize.
 * \param c             A channel end that is already allocated and connected to channel
 *                      end on the tile with which to establish an intertile link.
 *                      After this function returns, this channel end is no longer needed
 *                      and may be deallocated or used for other purposes.
 */
void rtos_intertile_init(
        rtos_intertile_t *intertile_ctx,
        chanend_t c);

/**@}*/

#endif /* RTOS_INTERTILE_H_ */
