// Copyright (c) 2021, XMOS Ltd, All rights reserved
#ifndef LEGACY_COMPAT_DEBUG_PRINTF
#define LEGACY_COMPAT_DEBUG_PRINTF

#ifdef RTOS_DEBUG_PRINTF_REMAP
#undef RTOS_DEBUG_PRINTF_REMAP
#endif
#define RTOS_DEBUG_PRINTF_REMAP 1

#include "rtos_printf.h"

#endif /* LEGACY_COMPAT_DEBUG_PRINTF */
