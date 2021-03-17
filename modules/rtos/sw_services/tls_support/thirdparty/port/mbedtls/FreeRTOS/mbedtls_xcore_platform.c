// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h> /* For mbedtls_platform_zeroize() */

extern struct tm *gmtime_r(const long *, struct tm *);

void *freertos_calloc( size_t n, size_t size )
{
    void* ptr;

    ptr = pvPortMalloc( n * size );

    if( ptr != NULL )
    {
        mbedtls_platform_zeroize( ptr, n * size );
    }

    return ptr;
}

struct tm *mbedtls_platform_gmtime_r(
        const mbedtls_time_t *tt,
        struct tm *tm_buf)
{
    struct tm *info;

    info =  gmtime_r(tt, tm_buf);

    // rtos_printf( "mbed gmtime: %d/%d/%02d %2d:%02d:%02d\n",
    //              (int)info->tm_mday,
    //              (int)info->tm_mon + 1,
    //              (int)info->tm_year + 1900,
    //              (int)info->tm_hour,
    //              (int)info->tm_min,
    //              (int)info->tm_sec);

    return info;
}

mbedtls_time_t mbedtls_platform_time(
        mbedtls_time_t* time)
{
    rtos_time_t now;
    now = rtos_time_get();
    return ( mbedtls_time_t ) now.seconds;
}
