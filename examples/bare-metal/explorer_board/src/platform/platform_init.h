// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef PLATFORM_INIT_H_
#define PLATFORM_INIT_H_

#include <xcore/chanend.h>

void platform_init_tile_0(chanend_t c_other_tile);
void platform_init_tile_1(chanend_t c_other_tile);

#endif /* PLATFORM_INIT_H_ */
