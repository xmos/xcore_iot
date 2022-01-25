// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "driver_instances.h"

static void clock_control_start(void)
{
    rtos_clock_control_rpc_config(cc_ctx_t0, appconfCLOCK_CONTROL_PORT, appconfCLOCK_CONTROL_RPC_HOST_PRIORITY);

#if ON_TILE(0)
    rtos_clock_control_start(cc_ctx_t0);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);

    clock_control_start();
}
