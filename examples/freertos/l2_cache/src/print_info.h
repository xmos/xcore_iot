// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef PRINT_INFO_H_
#define PRINT_INFO_H_

#include <stdint.h>
#include "xcore_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void print_info(uint32_t timer_ticks);

#ifdef __cplusplus
}
#endif

#endif // PRINT_INFO_H_
