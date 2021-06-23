// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_TEST_UTILS_H_
#define RTOS_TEST_UTILS_H_

#include "rtos_printf.h"

#define module_printf( testname, FMT, ... )   rtos_printf( "Tile[%d]|FCore[%d]|%u|%s|" FMT "\n", THIS_XCORE_TILE, rtos_core_id_get(), xscope_gettime(), testname, ##__VA_ARGS__ )

#define test_printf( FMT, ... )      module_printf("TEST", FMT, ##__VA_ARGS__)

#include "xcore/channel.h"
/* Chan transaction used to sync tiles */
#if ON_TILE(0)
#define sync(chan)     { chan_out_byte(chan, 0);   }
#else
#define sync(chan)     { (void)chan_in_byte(chan); }
#endif

#endif /* RTOS_TEST_UTILS_H_ */
