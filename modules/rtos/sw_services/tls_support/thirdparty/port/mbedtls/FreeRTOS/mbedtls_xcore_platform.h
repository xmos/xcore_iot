// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MBEDTLS_XCORE_PLATFORM_H
#define MBEDTLS_XCORE_PLATFORM_H

#include <FreeRTOS.h> /* For rtos_printf and vPortFree() */

/**
 * Calloc in FreeRTOS
 *
 * \param[in]     n              Number of objects
 * \param[in]     size           Size of each object
 *
 * \returns       return pointer to region of n*size bytes, initialized to all 0x00
 */
void *freertos_calloc(size_t n, size_t size);

struct tm *mbedtls_platform_gmtime_r(
        const mbedtls_time_t *tt,
        struct tm *tm_buf);

mbedtls_time_t mbedtls_platform_time(
        mbedtls_time_t* time);

#endif /* MBEDTLS_XCORE_PLATFORM_H */
