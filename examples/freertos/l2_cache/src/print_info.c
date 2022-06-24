// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h> // for PLATFORM_REFERENCE_MHZ
#include <stdio.h>
#include <stdlib.h>

#include "l2_cache.h"
#include "print_info.h"
#include "xcore_utils.h"

void print_info(uint32_t timer_ticks)
{
#if PRINT_TIMING_INFO
#if L2_CACHE_DEBUG_FLOAT_ON
    // NOTE: This is invalid if L2 Cache debug is enabled, so don't print it.
    printf("  Timing:   %0.01f us\n", timer_ticks / 100.0f);
#else
    debug_printf("  Timing:   %lu us\n", timer_ticks / 100);
#endif
#endif // PRINT_TIMING_INFO

#if L2_CACHE_DEBUG_ON
    // NOTE: This info isn't collected unless L2 Cache debug is enabled.
#if L2_CACHE_DEBUG_FLOAT_ON
    printf("  Cache Hit Rate: %0.04f\n", l2_cache_debug_hit_rate() );
#else
    debug_printf("  Cache Hit Rate: %lu\n", l2_cache_debug_hit_rate() );
#endif
    debug_printf("    Hit Count: %lu\n", get_hit_count() );
    debug_printf("    Fill Request Count: %lu\n", get_fill_request_count() );

    l2_cache_debug_stats_reset();
#endif // L2_CACHE_DEBUG_ON
}
