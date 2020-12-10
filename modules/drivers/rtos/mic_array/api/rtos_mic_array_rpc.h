// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_MIC_ARRAY_RPC_H_
#define RTOS_MIC_ARRAY_RPC_H_

void rtos_mic_array_rpc_client_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx);

void rtos_mic_array_rpc_host_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count);

void rtos_mic_array_rpc_config(
        rtos_mic_array_t *mic_array_ctx,
        unsigned intertile_port,
        unsigned host_task_priority);

#endif /* RTOS_MIC_ARRAY_RPC_H_ */
