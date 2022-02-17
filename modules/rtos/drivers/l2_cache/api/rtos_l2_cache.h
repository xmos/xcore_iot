// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_L2_CACHE_H_
#define RTOS_L2_CACHE_H_

/**
 * \addtogroup rtos_l2_cache_driver rtos_l2_cache_driver
 *
 * The public API for using the RTOS software memory driver.
 * @{
 */

#include <xcore/channel.h>

#include "rtos_osal.h"
#include "l2_cache.h"

/**
 * Convenience macro that may be used to specify the direct map cache to
 * rtos_l2_cache_init() in place of setup_fn and thread_fn.
 */
#define RTOS_L2_CACHE_DIRECT_MAP            l2_cache_setup_direct_map, l2_cache_direct_map

/**
 * Convenience macro that may be used to specify the two way associative cache to
 * rtos_l2_cache_init() in place of setup_fn and thread_fn.
 */
#define RTOS_L2_CACHE_TWO_WAY_ASSOCIATIVE   l2_cache_setup_two_way, l2_cache_two_way

/**
 * Convenience macro that may be used to specify the size of the cache buffer
 * for a direct map cache.
 * A pointer to the buffer of size RTOS_L2_CACHE_BUFFER_WORDS_DIRECT_MAP
 * should be passed to the cache_buffer argument of rtos_l2_cache_init().
 */
#define RTOS_L2_CACHE_BUFFER_WORDS_DIRECT_MAP L2_CACHE_BUFFER_WORDS_DIRECT_MAP(L2_CACHE_LINE_COUNT, L2_CACHE_LINE_SIZE_BYTES)

/**
 * Convenience macro that may be used to specify the size of the cache buffer
 * for a two way associative cache.
 * A pointer to the buffer of size RTOS_L2_CACHE_BUFFER_WORDS_TWO_WAY
 * should be passed to the cache_buffer argument of rtos_l2_cache_init().
 */
#define RTOS_L2_CACHE_BUFFER_WORDS_TWO_WAY L2_CACHE_BUFFER_WORDS_TWO_WAY(L2_CACHE_LINE_COUNT, L2_CACHE_LINE_SIZE_BYTES)

/**
 * Typedef to the RTOS l2 cache driver instance struct.
 */
typedef struct rtos_l2_cache_struct rtos_l2_cache_t;

/**
 * Struct representing an RTOS l2 cache driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_l2_cache_struct {
    L2_CACHE_SETUP_FN_ATTR l2_cache_setup_fn l2_cache_setup;
    L2_CACHE_THREAD_FN_ATTR l2_cache_thread_fn l2_cache_thread;
    L2_CACHE_SWMEM_READ_FN l2_cache_swmem_read_fn read_func;
    rtos_osal_thread_t hil_thread;
    chanend_t c_start;
    chanend_t c_thread;
    void* cache_buffer;
};

/**
 * Starts the RTOS l2 cache memory driver.
 */
void rtos_l2_cache_start(rtos_l2_cache_t* ctx);

/**
 * Initializes the l2 cache for use by the RTOS l2 cache memory driver.
 *
 * Cache buffer must be dword aligned
 */
void rtos_l2_cache_init(
    rtos_l2_cache_t* ctx,
    l2_cache_setup_fn setup_fn,
    l2_cache_thread_fn thread_fn,
    l2_cache_swmem_read_fn read_func,
    uint32_t io_core_mask,
    void* cache_buffer);

/**@}*/

#endif /* RTOS_L2_CACHE_H_ */
