// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_DRIVER_RPC_H_
#define RTOS_DRIVER_RPC_H_

#define RTOS_DRIVER_RPC_MAX_CLIENT_TILES 3

#include "rtos_intertile.h"

/**
 * Typedef to the RTOS driver RPC configuration struct.
 */
typedef struct rtos_driver_rpc_struct rtos_driver_rpc_t;

/**
 * Struct representing an RTOS driver RPC configuration.
 *
 * This struct is intended for use by the RTOS drivers to implement
 * their intertile RPC support. The members in this struct should not
 * be accessed directly by applications.
 */
struct rtos_driver_rpc_struct {
    union {
        rtos_intertile_address_t client_address[RTOS_DRIVER_RPC_MAX_CLIENT_TILES];
        struct {
            rtos_intertile_address_t host_address;
            void *host_ctx_ptr;
        };
    };

    size_t remote_client_count; /* This must be > 0 on the host. It must be 0 on the client */
    int host_task_priority; /* TODO: Consider renaming to rpc_task_priority. Could be used by client as well. */

    __attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
    void (*rpc_host_start)(rtos_driver_rpc_t *rpc_config); /* TODO: Consider renaming to rpc_task_start(). Could be used by client as well. */
};

#endif /* RTOS_DRIVER_RPC_H_ */
