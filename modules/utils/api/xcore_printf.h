// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_UTILS_PRINTF_H_
#define XCORE_UTILS_PRINTF_H_

/**
Debug Printing Module
=====================

This module provides a lightweight printf function that can
be enabled or disabled via configuration macros. Code can be declared
to be within a "debug unit" (usually a module) and prints can be
enabled/disabled per debug unit.

This uses the same DEBUG macros as lib_logging to control "debug units".
It also provides sprintf and snprintf functions.

**/

/**
 * Remaps all calls to debug_printf() to xcore_utils_printf().
 * When this is on, files should not include both xcore_utils.h
 * and debug_print.h.
 */
#define XCORE_UTILS_DEBUG_PRINTF_REMAP 1


#ifdef configENABLE_DEBUG_PRINTF
	#if configENABLE_DEBUG_PRINTF

		/* ensure that debug_printf is enabled */
		#ifdef DEBUG_PRINT_ENABLE
			#undef DEBUG_PRINT_ENABLE
		#endif
		#define DEBUG_PRINT_ENABLE 1

	#else /* configENABLE_DEBUG_PRINTF */

		/* ensure that debug_printf is disabled */
		#ifdef DEBUG_UNIT
			#undef DEBUG_UNIT
		#endif
		#ifdef DEBUG_PRINT_ENABLE
			#undef DEBUG_PRINT_ENABLE
		#endif

		#define DEBUG_PRINT_ENABLE 0

	#endif /* configENABLE_DEBUG_PRINTF */
#endif

#ifndef XCORE_UTILS_DEBUG_PRINTF_REMAP
#define XCORE_UTILS_DEBUG_PRINTF_REMAP 0
#endif

/* remap calls to debug_printf to xcore_utils_printf */
#if XCORE_UTILS_DEBUG_PRINTF_REMAP

    #ifdef _debug_printf_h_
    #error Do not include debug_print.h when using xcore_utils_printf
    #endif

    /* Ensure that if debug_print.h is included later that it is ignored */
    #define _debug_printf_h_

    #define debug_printf xcore_utils_printf

#endif /* XCORE_UTILS_DEBUG_PRINTF_REMAP */

#ifndef DEBUG_UNIT
#define DEBUG_UNIT APPLICATION
#endif

#ifndef DEBUG_PRINT_ENABLE_ALL
#define DEBUG_PRINT_ENABLE_ALL 0
#endif

#ifndef DEBUG_PRINT_ENABLE
#define DEBUG_PRINT_ENABLE 0
#endif

#if !defined(DEBUG_PRINT_ENABLE_APPLICATION) && !defined(DEBUG_PRINT_DISABLE_APPLICATION)
#define DEBUG_PRINT_ENABLE_APPLICATION DEBUG_PRINT_ENABLE
#endif

#define DEBUG_UTILS_JOIN0(x,y) x ## y
#define DEBUG_UTILS_JOIN(x,y) DEBUG_UTILS_JOIN0(x,y)

#if DEBUG_UTILS_JOIN(DEBUG_PRINT_ENABLE_,DEBUG_UNIT)
#define DEBUG_PRINT_ENABLE0 1
#endif

#if DEBUG_UTILS_JOIN(DEBUG_PRINT_DISABLE_,DEBUG_UNIT)
#define DEBUG_PRINT_ENABLE0 0
#endif

#if !defined(DEBUG_PRINT_ENABLE0)
#define DEBUG_PRINT_ENABLE0 DEBUG_PRINT_ENABLE_ALL
#endif

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

/**
 * Just like snprintf, but not all of the
 * standard C format control are supported.
 */
int xcore_utils_snprintf(char *str, size_t size, const char *fmt, ...);

/**
 * Just like sprintf, but not all of the
 * standard C format control are supported.
 */
int xcore_utils_sprintf(char *str, const char *fmt, ...);

/**
 * Just like vprintf, but not all of the
 * standard C format control are supported.
 */
#ifndef __XC__
int xcore_utils_vprintf(const char *fmt, va_list ap);
#endif

/**
 * Just like printf, but not all of the
 * standard C format control are supported.
 */
int xcore_utils_printf(const char *fmt, ...);

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#if DEBUG_PRINT_ENABLE0
#define xcore_utils_vprintf(...) xcore_utils_vprintf(__VA_ARGS__)
#define xcore_utils_printf(...)  xcore_utils_printf(__VA_ARGS__)
#else
#define xcore_utils_vprintf(...)
#define xcore_utils_printf(...)
#endif

#endif /* XCORE_UTILS_PRINTF_H_ */
