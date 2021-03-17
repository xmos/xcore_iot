// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef LEGACY_COMPAT_DEBUG_PRINTF
#define LEGACY_COMPAT_DEBUG_PRINTF

#ifdef RTOS_DEBUG_PRINTF_REMAP
#undef RTOS_DEBUG_PRINTF_REMAP
#endif
#define RTOS_DEBUG_PRINTF_REMAP 1

#include "rtos_printf.h"

#endif /* LEGACY_COMPAT_DEBUG_PRINTF */
