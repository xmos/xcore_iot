// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "driver_instances.h"

static rtos_intertile_t intertile_ctx_s;
rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

static rtos_clock_control_t cc_ctx_t0_s;
rtos_clock_control_t *cc_ctx_t0 = &cc_ctx_t0_s;
