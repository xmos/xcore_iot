// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_SPI_MASTER_RPC_H_
#define RTOS_SPI_MASTER_RPC_H_

void rtos_spi_master_rpc_client_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx);

void rtos_spi_master_rpc_host_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count);

void rtos_spi_master_rpc_config(
        rtos_spi_master_t *spi_master_ctx,
        unsigned intertile_port,
        unsigned host_task_priority);

#endif /* RTOS_SPI_MASTER_RPC_H_ */
