// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _rtos_printf_h_
#define _rtos_printf_h_

/**
Debug Printing Module
=====================

This module provides a lightweight RTOS safe printf function that can
be enabled or disabled via configuration macros. Code can be declared
to be within a "debug unit" (usually a module) and prints can be
enabled/disabled per debug unit.

This uses the same DEBUG macros as lib_logging to control "debug units",
but implements a slightly more capable and RTOS safe printf function.
It also provides sprintf and snprintf functions.

**/

#include "rtos_support_rtos_config.h"

#ifdef __rtos_support_conf_h_exists__
#include "rtos_support_conf.h"
#endif

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

#include "xcore_utils.h"

/**
 * Just like snprintf, but not all of the
 * standard C format control are supported.
 */
 #define rtos_snprintf  xcore_utils_snprintf

/**
 * Just like sprintf, but not all of the
 * standard C format control are supported.
 */
#define rtos_sprintf  xcore_utils_sprintf

/**
 * Just like vprintf, but not all of the
 * standard C format control are supported.
 */
#ifndef __XC__
#define rtos_vprintf  xcore_utils_vprintf
#endif

/**
 * Just like printf, but not all of the
 * standard C format control are supported.
 */
#define rtos_printf  xcore_utils_printf

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif // _rtos_printf_h_
