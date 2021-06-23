// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "board_init.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx)
{
    rtos_intertile_init(intertile_ctx, tile1);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile0);
}
