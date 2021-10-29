// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef PLATFORM_INIT_H_
#define PLATFORM_INIT_H_

#include <xcore/chanend.h>

/** TILE 0 Clock Blocks */
#define SPI_CLKBLK      XS1_CLKBLK_1
#define FLASH_CLKBLK    XS1_CLKBLK_2
#define MCLK_CLKBLK     XS1_CLKBLK_3
// #define UNUSED_CLKBLK     XS1_CLKBLK_4
// #define UNUSED_CLKBLK   XS1_CLKBLK_5

/** TILE 1 Clock Blocks */
#define PDM_CLKBLK_1    XS1_CLKBLK_1
#define PDM_CLKBLK_2    XS1_CLKBLK_2
#define I2S_CLKBLK      XS1_CLKBLK_3
// #define UNUSED_CLKBLK   XS1_CLKBLK_4
// #define UNUSED_CLKBLK   XS1_CLKBLK_5

void platform_init_tile_0(chanend_t c_other_tile);
void platform_init_tile_1(chanend_t c_other_tile);

#endif /* PLATFORM_INIT_H_ */
