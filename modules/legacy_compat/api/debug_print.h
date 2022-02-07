// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef LEGACY_COMPAT_DEBUG_PRINTF
#define LEGACY_COMPAT_DEBUG_PRINTF

#ifdef XCORE_UTILS_DEBUG_PRINTF_REMAP
#undef XCORE_UTILS_DEBUG_PRINTF_REMAP
#endif
#define XCORE_UTILS_DEBUG_PRINTF_REMAP 1

#ifdef __rtos_printf_h_exists__
#include "rtos_printf.h"
#else
#include "xcore_utils.h"
#define debug_printf    xcore_utils_printf
#endif

#endif /* LEGACY_COMPAT_DEBUG_PRINTF */
