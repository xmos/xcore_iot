// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MBEDTLS_XCORE_PLATFORM_H
#define MBEDTLS_XCORE_PLATFORM_H


struct tm *mbedtls_platform_gmtime_r(
        const mbedtls_time_t *tt,
        struct tm *tm_buf);

mbedtls_time_t mbedtls_platform_time(
        mbedtls_time_t* time);

#endif /* MBEDTLS_XCORE_PLATFORM_H */
