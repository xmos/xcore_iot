// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef TEST_PRINT_H_
#define TEST_PRINT_H_

#include "rtos_printf.h"

#define module_printf( testname, FMT, ... ) rtos_printf( "Tile[%d]|FCore[%d]|%d|%s|" FMT "\n", THIS_XCORE_TILE, rtos_core_id_get(), xscope_gettime(), testname, ##__VA_ARGS__ )

#define test_printf( FMT, ... )      module_printf("Test", FMT, ##__VA_ARGS__)
#define kernel_printf( FMT, ... )    module_printf("Kernel", FMT, ##__VA_ARGS__)

#endif /* TEST_PRINT_H_ */
