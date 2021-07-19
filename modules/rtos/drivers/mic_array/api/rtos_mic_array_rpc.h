// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_MIC_ARRAY_RPC_H_
#define RTOS_MIC_ARRAY_RPC_H_

/**
 * \addtogroup rtos_mic_array_driver rtos_mic_array_driver
 * @{
 */

/**
 * \addtogroup rtos_mic_array_driver_rpc rtos_mic_array_driver_rpc
 *
 * The functions for setting up RPC with an RTOS mic array driver instance.
 * @{
 */

/**
 * Initializes an RTOS mic array driver instance on a client tile.
 * This allows a tile that does not own the actual driver instance
 * to use a driver instance on another tile. This will be called
 * instead of rtos_mic_array_init(). The host tile that owns the actual
 * instance must simultaneously call rtos_mic_array_rpc_host_init().
 *
 * \param mic_array_ctx      A pointer to the mic array driver instance to initialize.
 * \param rpc_config         A pointer to an RPC config struct. This must have
 *                           the same scope as \p mic_array_ctx.
 * \param host_intertile_ctx A pointer to the intertile driver instance to use
 *                           for performing the communication between the client
 *                           and host tiles. This must have the same scope as
 *                           \p mic_array_ctx.
 */
void rtos_mic_array_rpc_client_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx);

/**
 * Performs additional initialization on a mic array driver instance to
 * allow client tiles to use the mic array driver instance. Each client
 * tile that will use this instance must simultaneously call
 * rtos_mic_array_rpc_client_init().
 *
 * \param mic_array_ctx        A pointer to the mic array driver instance to share with clients.
 * \param rpc_config           A pointer to an RPC config struct. This must have
 *                             the same scope as \p mic_array_ctx.
 * \param client_intertile_ctx An array of pointers to the intertile driver instances to
 *                             use for performing the communication between the host tile
 *                             and each client tile. This must have the same scope as
 *                             \p mic_array_ctx.
 * \param remote_client_count  The number of client tiles to share this driver instance with.
 */
void rtos_mic_array_rpc_host_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count);

/**
 * Configures the RPC for a mic array driver instance. This must be called
 * by both the host tile and all client tiles.
 *
 * On the client tiles this must be called after calling rtos_mic_array_rpc_client_init().
 * After calling this, the client tile may immediately begin to call the core mic array
 * functions on this driver instance. It does not need to wait for the host to call
 * rtos_mic_array_start().
 *
 * On the host tile this must be called both after calling rtos_mic_array_rpc_host_init()
 * and before calling rtos_mic_array_start().
 *
 * \param mic_array_ctx      A pointer to the mic array driver instance to configure the RPC for.
 * \param intertile_port     The port number on the intertile channel to use for transferring
 *                           the RPC requests and responses for this driver instance. This port
 *                           must not be shared by any other functions. The port must be the same
 *                           for the host and all its clients.
 * \param host_task_priority The priority to use for the task on the host tile that handles RPC
 *                           requests from the clients.
 */
void rtos_mic_array_rpc_config(
        rtos_mic_array_t *mic_array_ctx,
        unsigned intertile_port,
        unsigned host_task_priority);

/**@}*/
/**@}*/

#endif /* RTOS_MIC_ARRAY_RPC_H_ */
