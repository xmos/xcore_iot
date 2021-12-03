// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h> /* For mbedtls_platform_zeroize() */

extern struct tm *gmtime_r(const long *, struct tm *);

struct tm *mbedtls_platform_gmtime_r(
        const mbedtls_time_t *tt,
        struct tm *tm_buf)
{
    struct tm *info;

    info =  gmtime_r(tt, tm_buf);

    return info;
}

mbedtls_time_t mbedtls_platform_time(
        mbedtls_time_t* time)
{
    // FIXME
    return ( mbedtls_time_t ) 0;
}
