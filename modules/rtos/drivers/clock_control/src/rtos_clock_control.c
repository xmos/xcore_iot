// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/assert.h>

#include "rtos_clock_control.h"

__attribute__((fptrgroup("rtos_clock_control_set_ref_clk_div_fptr_grp")))
static void clock_control_local_set_ref_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    set_local_node_ref_clk_div(divider);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_set_processor_clk_div_fptr_grp")))
static void clock_control_local_set_processor_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    set_local_tile_processor_clk_div(divider);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_set_switch_clk_div_fptr_grp")))
static void clock_control_local_set_switch_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    set_local_node_switch_clk_div(divider);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_get_ref_clk_div_fptr_grp")))
static unsigned clock_control_local_get_ref_clk_div(
        rtos_clock_control_t *ctx)
{
    return get_local_node_ref_clk_div();
}

__attribute__((fptrgroup("rtos_clock_control_get_processor_clk_div_fptr_grp")))
static unsigned clock_control_local_get_processor_clk_div(
        rtos_clock_control_t *ctx)
{
    return get_local_tile_processor_clk_div();
}

__attribute__((fptrgroup("rtos_clock_control_get_switch_clk_div_fptr_grp")))
static unsigned clock_control_local_get_switch_clk_div(
        rtos_clock_control_t *ctx)
{
    return get_local_node_switch_clk_div();
}

__attribute__((fptrgroup("rtos_clock_control_get_processor_clock_fptr_grp")))
static unsigned clock_control_local_get_processor_clock(
        rtos_clock_control_t *ctx)
{
    return get_local_core_clock();
}

__attribute__((fptrgroup("rtos_clock_control_get_ref_clock_fptr_grp")))
static unsigned clock_control_local_get_ref_clock(
        rtos_clock_control_t *ctx)
{
    return get_local_ref_clock();
}

__attribute__((fptrgroup("rtos_clock_control_get_switch_clock_fptr_grp")))
static unsigned clock_control_local_get_switch_clock(
        rtos_clock_control_t *ctx)
{
    return get_local_switch_clock();
}

__attribute__((fptrgroup("rtos_clock_control_set_node_pll_ratio_fptr_grp")))
static void clock_control_local_set_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned pre_div,
        unsigned mul,
        unsigned post_div)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    set_local_node_pll_ratio(pre_div, mul, post_div);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_get_node_pll_ratio_fptr_grp")))
static void clock_control_local_get_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned *pre_div,
        unsigned *mul,
        unsigned *post_div)
{
    get_local_node_pll_ratio(pre_div, mul, post_div);
}

__attribute__((fptrgroup("rtos_clock_control_scale_links_fptr_grp")))
static void clock_control_local_scale_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr,
        unsigned delay_intra,
        unsigned delay_inter)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    scale_links(start_addr, end_addr, delay_intra, delay_inter);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_reset_links_fptr_grp")))
static void clock_control_local_reset_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    reset_local_links(start_addr, end_addr);
    rtos_osal_mutex_put(&ctx->lock);
}

__attribute__((fptrgroup("rtos_clock_control_get_local_lock_fptr_grp")))
static void clock_control_local_lock(
        rtos_clock_control_t *ctx)
{
    mrsw_lock_reader_get(&ctx->local_lock, RTOS_OSAL_PORT_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_clock_control_release_local_lock_fptr_grp")))
static void clock_control_local_unlock(
        rtos_clock_control_t *ctx)
{
    mrsw_lock_reader_put(&ctx->local_lock);
}

void rtos_clock_control_start(
        rtos_clock_control_t *ctx)
{
    mrsw_lock_create(&ctx->local_lock, "clkctrl_lock", MRSW_WRITER_PREFERRED);
    rtos_osal_mutex_create(&ctx->lock, "clock_control_lock", RTOS_OSAL_RECURSIVE);

    if (ctx->rpc_config != NULL && ctx->rpc_config->rpc_host_start != NULL) {
        ctx->rpc_config->rpc_host_start(ctx->rpc_config);
    }
}

void rtos_clock_control_init(
        rtos_clock_control_t *ctx)
{
    enable_local_tile_processor_clock_divider();

    ctx->set_ref_clk_div = clock_control_local_set_ref_clk_div;
    ctx->set_processor_clk_div = clock_control_local_set_processor_clk_div;
    ctx->set_switch_clk_div = clock_control_local_set_switch_clk_div;
    ctx->get_ref_clk_div = clock_control_local_get_ref_clk_div;
    ctx->get_processor_clk_div = clock_control_local_get_processor_clk_div;
    ctx->get_switch_clk_div = clock_control_local_get_switch_clk_div;
    ctx->get_processor_clock = clock_control_local_get_processor_clock;
    ctx->get_ref_clock = clock_control_local_get_ref_clock;
    ctx->get_switch_clock = clock_control_local_get_switch_clock;
    ctx->set_node_pll_ratio = clock_control_local_set_node_pll_ratio;
    ctx->get_node_pll_ratio = clock_control_local_get_node_pll_ratio;
    ctx->scale_links = clock_control_local_scale_links;
    ctx->reset_links = clock_control_local_reset_links;
    ctx->get_local_lock = clock_control_local_lock;
    ctx->release_local_lock = clock_control_local_unlock;
}
