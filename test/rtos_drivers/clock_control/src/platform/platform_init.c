// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "driver_instances.h"

static void clock_control_init(void)
{
    static rtos_driver_rpc_t clock_control_rpc_config_t0;

#if ON_TILE(0)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_clock_control_init(cc_ctx_t0);

    rtos_clock_control_rpc_host_init(
            cc_ctx_t0,
            &clock_control_rpc_config_t0,
            client_intertile_ctx,
            1);
#endif

#if ON_TILE(1)
    rtos_clock_control_rpc_client_init(
            cc_ctx_t0,
            &clock_control_rpc_config_t0,
            intertile_ctx);
#endif
}


void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    clock_control_init();
}
