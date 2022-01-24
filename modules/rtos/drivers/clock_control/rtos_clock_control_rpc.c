// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/assert.h>

#include "rtos/drivers/clock_control/api/rtos_clock_control.h"
#include "rtos/sw_services/concurrency_support/api/mrsw_lock.h"
#include "xcore_clock_control.h"

void rtos_clock_control_rpc_client_init(
        rtos_clock_control_t *cc_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    ;
}

void rtos_clock_control_rpc_config(
        rtos_clock_control_t *cc_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    ;
}

void rtos_clock_control_rpc_host_init(
        rtos_clock_control_t *cc_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    ;
}
