// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SWMEM_MACROS_H_
#define SWMEM_MACROS_H_

#include <stdint.h>

#ifdef USE_SWMEM

#define XCORE_CODE_SECTION_ATTRIBUTE    __attribute__((section(".SwMem_code")))
#define XCORE_DATA_SECTION_ATTRIBUTE    __attribute__((section(".SwMem_data")))

#else

#define XCORE_CODE_SECTION_ATTRIBUTE
#define XCORE_DATA_SECTION_ATTRIBUTE

#endif // USE_SWMEM

#endif // SWMEM_MACROS_H_
