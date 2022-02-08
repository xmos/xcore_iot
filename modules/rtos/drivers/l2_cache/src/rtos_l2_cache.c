// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/channel.h>

#include "rtos_interrupt.h"

#include "rtos_osal.h"
#include "rtos_l2_cache.h"
#include "l2_cache.h"

static void rtos_l2_cache_thread(rtos_l2_cache_t *ctx)
{
    L2_CACHE_THREAD_FN_ATTR l2_cache_thread_fn thread_fn;
    thread_fn = ctx->l2_cache_thread;

    L2_CACHE_SETUP_FN_ATTR l2_cache_setup_fn setup_func;
    setup_func = ctx->l2_cache_setup;

    (void) s_chan_in_byte(ctx->c_thread);

    rtos_printf("L2 Cache on core %d\n", rtos_core_id_get());

    setup_func(L2_CACHE_LINE_COUNT,
               L2_CACHE_LINE_SIZE_BYTES,
               ctx->cache_buffer,
               (l2_cache_swmem_read_fn)ctx->read_func);

    /* This call should never return */
    thread_fn(NULL);

    vTaskDelete(NULL);
}

void rtos_l2_cache_start(rtos_l2_cache_t *ctx)
{
    /* Tells the I/O thread to enter the l2 cache thread function */
    s_chan_out_byte(ctx->c_start, 0);
}

void rtos_l2_cache_init(
    rtos_l2_cache_t* ctx,
    l2_cache_setup_fn setup_fn,
    l2_cache_thread_fn thread_fn,
    l2_cache_swmem_read_fn read_func,
    uint32_t io_core_mask,
    void* cache_buffer)
{
    memset(ctx, 0, sizeof(rtos_l2_cache_t));

    channel_t c_tmp = chan_alloc();
    xassert(c_tmp.end_a != 0);

    ctx->c_start = c_tmp.end_a;
    ctx->c_thread = c_tmp.end_b;
    ctx->l2_cache_setup = setup_fn;
    ctx->l2_cache_thread = thread_fn;
    ctx->cache_buffer = cache_buffer;
    ctx->read_func = read_func;

    rtos_osal_thread_create(
            &ctx->hil_thread,
            "l2_cache_thread",
            (rtos_osal_entry_function_t) rtos_l2_cache_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(rtos_l2_cache_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the L2 cache thread is never preempted */
    rtos_osal_thread_preemption_disable(&ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&ctx->hil_thread, ~io_core_mask);
}
