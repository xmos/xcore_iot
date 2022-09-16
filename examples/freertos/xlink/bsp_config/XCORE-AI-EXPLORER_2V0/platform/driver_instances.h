// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_intertile.h"
#include "rtos_i2c_master.h"
#include "rtos_gpio.h"

#define I2C_TILE_NO        0

/** TILE 0 Clock Blocks */
// #define UNUSED_CLKBLK  XS1_CLKBLK_1
// #define UNUSED_CLKBLK   XS1_CLKBLK_2
// #define UNUSED_CLKBLK    XS1_CLKBLK_3
// #define UNUSED_CLKBLK  XS1_CLKBLK_4 /* Reserved for lib_xud */
// #define UNUSED_CLKBLK  XS1_CLKBLK_5 /* Reserved for lib_xud */

/** TILE 1 Clock Blocks */
// #define UNUSED_CLKBLK  XS1_CLKBLK_1
// #define UNUSED_CLKBLK  XS1_CLKBLK_2
// #define UNUSED_CLKBLK    XS1_CLKBLK_3
// #define UNUSED_CLKBLK XS1_CLKBLK_4
// #define UNUSED_CLKBLK XS1_CLKBLK_5

extern rtos_intertile_t *intertile_ctx;
extern rtos_gpio_t *gpio_ctx_t0;
extern rtos_gpio_t *gpio_ctx_t1;
extern rtos_i2c_master_t *i2c_master_ctx;

#endif /* DRIVER_INSTANCES_H_ */
