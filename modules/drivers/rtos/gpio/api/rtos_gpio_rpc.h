// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_GPIO_RPC_H_
#define RTOS_GPIO_RPC_H_

void rtos_gpio_rpc_client_init(
        rtos_gpio_t *i2c_master_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx);

void rtos_gpio_rpc_host_init(
        rtos_gpio_t *i2c_master_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count);

void rtos_gpio_rpc_config(
        rtos_gpio_t *i2c_master_ctx,
        unsigned intertile_port,
        unsigned host_task_priority);

#endif /* RTOS_GPIO_RPC_H_ */
