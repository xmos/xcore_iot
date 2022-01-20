// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/assert.h>

#include "rtos/drivers/clock_control/api/rtos_clock_control.h"
#include "xcore_clock_control.h"

__attribute__((fptrgroup("rtos_clock_control_set_ref_clk_div_fptr_grp")))
static void rtos_local_clock_control_set_ref_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    ;
}

// __attribute__((fptrgroup("rtos_clock_control_set_processor_clk_div_fptr_grp")))
// void (*set_processor_clk_div)(rtos_clock_control_t *, unsigned);
//
// __attribute__((fptrgroup("rtos_clock_control_set_switch_clk_div_fptr_grp")))
// void (*set_switch_clk_div)(rtos_clock_control_t *, unsigned);
//
// __attribute__((fptrgroup("rtos_clock_control_get_ref_clk_div_fptr_grp")))
// unsigned (*get_ref_clk_div)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_get_processor_clk_div_fptr_grp")))
// unsigned (*get_processor_clk_div)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_get_switch_clk_div_fptr_grp")))
// unsigned (*get_switch_clk_div)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_get_processor_clock_fptr_grp")))
// unsigned (*get_processor_clock)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_get_ref_clock_fptr_grp")))
// unsigned (*get_ref_clock)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_get_switch_clock_fptr_grp")))
// unsigned (*get_switch_clock)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_set_node_pll_ratio_fptr_grp")))
// void (*set_node_pll_ratio)(rtos_clock_control_t *, unsigned, unsigned, unsigned);
//
// __attribute__((fptrgroup("rtos_clock_control_get_node_pll_ratio_fptr_grp")))
// void (*get_node_pll_ratio)(rtos_clock_control_t *, unsigned *, unsigned *, unsigned *);
//
// __attribute__((fptrgroup("rtos_clock_control_scale_links_fptr_grp")))
// void (*scale_links)(rtos_clock_control_t *, unsigned, unsigned, unsigned, unsigned);
//
// __attribute__((fptrgroup("rtos_clock_control_reset_links_fptr_grp")))
// void (*reset_links)(rtos_clock_control_t *, unsigned, unsigned);
//
// __attribute__((fptrgroup("rtos_clock_control_get_local_lock_fptr_grp")))
// void (*get_local_lock)(rtos_clock_control_t *);
//
// __attribute__((fptrgroup("rtos_clock_control_release_local_lock_fptr_grp")))
// void (*release_local_lock)(rtos_clock_control_t *);


// void rtos_clock_control_set_processor_clk_div(
//         rtos_clock_control_t *ctx,
//         unsigned divider)
// {
//     ;
// }
//
// void rtos_clock_control_set_switch_clk_div(
//         rtos_clock_control_t *ctx,
//         unsigned divider)
// {
//     ;
// }
//
// void rtos_clock_control_scale_links(
//         rtos_clock_control_t *ctx,
//         unsigned delay_intra,
//         unsigned delay_inter,
//         unsigned start_addr,
//         unsigned end_addr)
// {
//     ;
// }
//
// void rtos_clock_control_reset_links(
//         rtos_clock_control_t *ctx,
//         unsigned start_addr,
//         unsigned end_addr)
// {
//     ;
// }

void rtos_clock_control_start(
        rtos_clock_control_t *ctx)
{
    ;
}

void rtos_clock_control_init(
        rtos_clock_control_t *ctx)
{
    enable_local_tile_processor_clock_divider();
    ;
}
