// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_CLOCK_CONTROL_H_
#define RTOS_CLOCK_CONTROL_H_

/**
 * \addtogroup rtos_clock_control_driver rtos_clock_control_driver
 *
 * The public API for using the RTOS clock control driver.
 * @{
 */

#include "xcore_clock_control.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"
#include "mrsw_lock.h"

/**
 * Typedef to the RTOS Clock Control driver instance struct.
 */
typedef struct rtos_clock_control_struct rtos_clock_control_t;

/**
 * Struct representing an RTOS clock control driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_clock_control_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_clock_control_set_ref_clk_div_fptr_grp")))
    void (*set_ref_clk_div)(rtos_clock_control_t *, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_set_processor_clk_div_fptr_grp")))
    void (*set_processor_clk_div)(rtos_clock_control_t *, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_set_switch_clk_div_fptr_grp")))
    void (*set_switch_clk_div)(rtos_clock_control_t *, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_get_ref_clk_div_fptr_grp")))
    unsigned (*get_ref_clk_div)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_get_processor_clk_div_fptr_grp")))
    unsigned (*get_processor_clk_div)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_get_switch_clk_div_fptr_grp")))
    unsigned (*get_switch_clk_div)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_get_processor_clock_fptr_grp")))
    unsigned (*get_processor_clock)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_get_ref_clock_fptr_grp")))
    unsigned (*get_ref_clock)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_get_switch_clock_fptr_grp")))
    unsigned (*get_switch_clock)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_set_node_pll_ratio_fptr_grp")))
    void (*set_node_pll_ratio)(rtos_clock_control_t *, unsigned, unsigned, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_get_node_pll_ratio_fptr_grp")))
    void (*get_node_pll_ratio)(rtos_clock_control_t *, unsigned *, unsigned *, unsigned *);

    __attribute__((fptrgroup("rtos_clock_control_scale_links_fptr_grp")))
    void (*scale_links)(rtos_clock_control_t *, unsigned, unsigned, unsigned, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_reset_links_fptr_grp")))
    void (*reset_links)(rtos_clock_control_t *, unsigned, unsigned);

    __attribute__((fptrgroup("rtos_clock_control_get_local_lock_fptr_grp")))
    void (*get_local_lock)(rtos_clock_control_t *);

    __attribute__((fptrgroup("rtos_clock_control_release_local_lock_fptr_grp")))
    void (*release_local_lock)(rtos_clock_control_t *);

    /**
     * MRSW lock used to provide concurrency between local and remote tiles.
     * Local lock uses are "readers".  Remote tile lock uses are as "writer"
     * Setup as writer preferred.
     */
    mrsw_lock_t local_lock;
    rtos_osal_mutex_t lock; /* Only used by RPC client */
};

#include "rtos_clock_control_rpc.h"

/**
 * \addtogroup rtos_clock_control_driver_core rtos_clock_control_driver_core
 *
 * The core functions for using an RTOS clock control driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Sets the reference clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 * \param divider  The value + 1 to write to XS1_SSWITCH_REF_CLK_DIVIDER_NUM
 */
inline void rtos_clock_control_set_ref_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    ctx->set_ref_clk_div(ctx, divider);
}

/**
 * Gets the reference clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_ref_clk_div(
        rtos_clock_control_t *ctx)
{
    return ctx->get_ref_clk_div(ctx);
}

/**
 * Sets the tile clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 * \param divider  The value + 1 to write to XS1_PSWITCH_PLL_CLK_DIVIDER_NUM
 */
inline void rtos_clock_control_set_processor_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    ctx->set_processor_clk_div(ctx, divider);
}

/**
 * Gets the tile clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_processor_clk_div(
        rtos_clock_control_t *ctx)
{
    return ctx->get_processor_clk_div(ctx);
}

/**
 * Sets the switch clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 * \param divider  The value + 1 to write to XS1_SSWITCH_CLK_DIVIDER_NUM
 */
inline void rtos_clock_control_set_switch_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    ctx->set_switch_clk_div(ctx, divider);
}

/**
 * Gets the switch clock divider register value for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_switch_clk_div(
        rtos_clock_control_t *ctx)
{
    return ctx->get_switch_clk_div(ctx);
}

/**
 * Gets the calculated reference clock frequency for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_ref_clock(
        rtos_clock_control_t *ctx)
{
    return ctx->get_ref_clock(ctx);
}

/**
 * Gets the calculated core clock frequency for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_processor_clock(
        rtos_clock_control_t *ctx)
{
    return ctx->get_processor_clock(ctx);
}

/**
 * Gets the calculated switch clock frequency for the tile that owns this
 * driver instance.
 *
 * \param ctx      A pointer to the clock control driver instance to use.
 */
inline unsigned rtos_clock_control_get_switch_clock(
        rtos_clock_control_t *ctx)
{
    return ctx->get_switch_clock(ctx);
}

/**
 * Sets the intra token delay and inter token delay to the xlinks within an
 * address range, inclusive, for the tile that owns this driver instance.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 * \param start_addr   The starting link address
 * \param end_addr     The ending address
 * \param delay_intra  The intra token delay value
 * \param delay_inter  The inter token delay value
 */
inline void rtos_clock_control_scale_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr,
        unsigned delay_intra,
        unsigned delay_inter)
{
    ctx->scale_links(ctx, start_addr, end_addr, delay_intra, delay_inter);
}

/**
 * Resets the xlinks within an address range, inclusive for the tile that
 * owns this driver instance.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 * \param start_addr   The starting link address
 * \param end_addr     The ending address
 */
inline void rtos_clock_control_reset_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr)
{
    ctx->reset_links(ctx, start_addr, end_addr);
}

/**
 * Sets the tile clock PLL control register value on the tile that owns this
 * driver instance. The value set is calculated from the divider stage 1,
 * multiplier stage, and divider stage 2 values provided.
 *
 * VCO freq = fosc * (F + 1) / (2 * (R + 1))
 * VCO must be between 260MHz and 1.3GHz for XS2
 * Core freq = VCO / (OD + 1)
 *
 * Refer to the XCore Clock Frequency Control document for more details.
 *
 * Note: This function will not reset the chip and wait for the PLL to settle
 * before re-enabling the chip to allow for large frequency jumps.  This will
 * cause a delay during settings.
 *
 * Note: It is up to the application to ensure that it is safe to change
 * the clock.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 * \param pre_div      The value of R
 * \param mul          The value of F
 * \param post_div     The value of OD
 */
inline void rtos_clock_control_set_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned pre_div,
        unsigned mul,
        unsigned post_div)
{
    ctx->set_node_pll_ratio(ctx, pre_div, mul, post_div);
}

/**
 * Gets the divider stage 1, multiplier stage, and divider stage 2 values
 * from the tile clock PLL control register values on the tile that owns this
 * driver instance.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 * \param pre_div      A pointer to be populated with the value of R
 * \param mul          A pointer to be populated with the value of F
 * \param post_div     A pointer to be populated with the value of OD
 */
inline void rtos_clock_control_get_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned *pre_div,
        unsigned *mul,
        unsigned *post_div)
{
    ctx->get_node_pll_ratio(ctx, pre_div, mul, post_div);
}

/**
 * Gets the local lock for clock control on the tile that owns this driver
 * instance.  This is intended for applications to use to prevent clock changes
 * around critical sections.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 */
inline void rtos_clock_control_get_local_lock(
        rtos_clock_control_t *ctx)
{
    ctx->get_local_lock(ctx);
}

/**
 * Releases the local lock for clock control on the tile that owns this driver
 * instance.
 *
 * \param ctx          A pointer to the clock control driver instance to use.
 */
inline void rtos_clock_control_release_local_lock(
        rtos_clock_control_t *ctx)
{
    ctx->release_local_lock(ctx);
}

/**@}*/

/**
 * Starts an RTOS clock control driver instance. This must only be called by the
 * tile that owns the driver instance. It may be called either before or after
 * starting the RTOS, but must be called before any of the core clock control
 * driver functions are called with this instance.
 *
 * rtos_clock_control_init() must be called on this clock control driver
 * instance prior to calling this.
 *
 * \param ctx A pointer to the clock control driver instance to start.
 */
void rtos_clock_control_start(
        rtos_clock_control_t *ctx);

/**
 * Initializes an RTOS clock control driver instance. There should only be one
 * per tile. This must only be called by the tile that owns the driver instance.
 * It may be called either before or after starting the RTOS, but must be called
 * before calling rtos_clock_control_start() or any of the core clock control
 * driver functions with this instance.
 *
 * \param ctx A pointer to the GPIO driver instance to initialize.
 */
void rtos_clock_control_init(
        rtos_clock_control_t *ctx);

/**@}*/

#endif /* RTOS_CLOCK_CONTROL_H_ */
