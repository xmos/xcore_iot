// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_SPI_MASTER_RPC_H_
#define RTOS_SPI_MASTER_RPC_H_

/**
 * \addtogroup rtos_spi_master_driver rtos_spi_master_driver
 * @{
 */

/**
 * \addtogroup rtos_spi_master_driver_rpc rtos_spi_master_driver_rpc
 *
 * The functions for setting up RPC with an RTOS SPI master driver instance.
 * @{
 */

/**
 * Initializes an RTOS SPI master driver instance on a client tile,
 * as well as any number of SPI device instances.
 * This allows a tile that does not own the actual driver instance
 * to use a driver instance on another tile. This will be called
 * instead of rtos_spi_master_init() and rtos_spi_master_device_init().
 * The host tile that owns the actual instances must simultaneously call
 * rtos_spi_master_rpc_host_init().
 *
 * \param spi_master_ctx     A pointer to the SPI master driver instance to initialize.
 * \param spi_device_ctx     An array of pointers to SPI device instances to initialize.
 * \param spi_device_count   The number of SPI device instances to initialize.
 * \param rpc_config         A pointer to an RPC config struct. This must have
 *                           the same scope as \p spi_master_ctx.
 * \param host_intertile_ctx A pointer to the intertile driver instance to use
 *                           for performing the communication between the client
 *                           and host tiles. This must have the same scope as
 *                           \p spi_master_ctx.
 */
void rtos_spi_master_rpc_client_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx);

/**
 * Performs additional initialization on a SPI master driver instance to
 * allow client tiles to use the SPI master driver instance. Each client
 * tile that will use this instance must simultaneously call
 * rtos_spi_master_rpc_client_init().
 *
 * \param spi_master_ctx       A pointer to the SPI master driver instance to share with clients.
 * \param spi_device_ctx       An array of pointers to SPI device instances to share with clients.
 * \param spi_device_count     The number of SPI device instances to share.
 * \param rpc_config           A pointer to an RPC config struct. This must have
 *                             the same scope as \p spi_master_ctx.
 * \param client_intertile_ctx An array of pointers to the intertile driver instances to
 *                             use for performing the communication between the host tile
 *                             and each client tile. This must have the same scope as
 *                             \p spi_master_ctx.
 * \param remote_client_count  The number of client tiles to share this driver instance with.
 */
void rtos_spi_master_rpc_host_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count);

/**
 * Configures the RPC for a SPI master driver instance. This must be called
 * by both the host tile and all client tiles.
 *
 * On the client tiles this must be called after calling rtos_spi_master_rpc_client_init().
 * After calling this, the client tile may immediately begin to call the core SPI master
 * functions on this driver instance. It does not need to wait for the host to call
 * rtos_spi_master_start().
 *
 * On the host tile this must be called both after calling rtos_spi_master_rpc_host_init()
 * and before calling rtos_spi_master_start().
 *
 * \param spi_master_ctx     A pointer to the SPI master driver instance to configure the RPC for.
 * \param intertile_port     The port number on the intertile channel to use for transferring
 *                           the RPC requests and responses for this driver instance. This port
 *                           must not be shared by any other functions. The port must be the same
 *                           for the host and all its clients.
 * \param host_task_priority The priority to use for the task on the host tile that handles RPC
 *                           requests from the clients.
 */
void rtos_spi_master_rpc_config(
        rtos_spi_master_t *spi_master_ctx,
        unsigned intertile_port,
        unsigned host_task_priority);

/**@}*/
/**@}*/

#endif /* RTOS_SPI_MASTER_RPC_H_ */
