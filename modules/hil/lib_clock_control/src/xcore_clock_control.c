// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>

#include "xcore_clock_control.h"
#include "xcore_utils.h"

void enable_local_tile_processor_clock_divider(void)
{
    unsigned xcore_ctrl0_data = getps(XS1_PS_XCORE_CTRL0);
    xcore_ctrl0_data = XS1_XCORE_CTRL0_CLK_DIVIDER_EN_SET(xcore_ctrl0_data, 1);
    setps(XS1_PS_XCORE_CTRL0, xcore_ctrl0_data);
}

void set_node_ref_clk_div(unsigned tileid, unsigned divider)
{
    unsigned reg_val = 0;
    do
    {
        (void) write_sswitch_reg(tileid, XS1_SSWITCH_REF_CLK_DIVIDER_NUM, divider - 1);
        (void) read_sswitch_reg(tileid, XS1_SSWITCH_REF_CLK_DIVIDER_NUM, &reg_val);
    } while((divider - 1) != reg_val);
}

unsigned get_node_ref_clk_div(unsigned tileid)
{
    unsigned reg_val = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_REF_CLK_DIVIDER_NUM, &reg_val);
    return reg_val + 1;
}

void set_node_switch_clk_div(unsigned tileid, unsigned divider)
{
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_CLK_DIVIDER_NUM, divider - 1);
}

unsigned get_node_switch_clk_div(unsigned tileid)
{
    unsigned reg_val = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_CLK_DIVIDER_NUM, &reg_val);
    return reg_val + 1;
}

void set_tile_processor_clk_div(unsigned tileid, unsigned divider)
{
    unsigned reg_val = 0;
    do
    {
      (void) write_pswitch_reg(tileid, XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, divider - 1);
      (void) read_pswitch_reg(tileid, XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, &reg_val);
    } while((divider - 1) != reg_val);
}

unsigned get_tile_processor_clk_div(unsigned tileid)
{
    unsigned reg_val= 0;
    (void) read_pswitch_reg(tileid, XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, &reg_val);
    return reg_val + 1;
}

void set_node_pll_reg(unsigned tileid, unsigned reg_val)
{
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_PLL_CTL_NUM, reg_val);
}

unsigned get_node_pll_reg(unsigned tileid)
{
    unsigned reg_val = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_PLL_CTL_NUM, &reg_val);
    return reg_val;
}

void set_node_pll_ratio(unsigned tileid, unsigned pre_div, unsigned mul, unsigned post_div)
{
    unsigned pll_val = 0;
    pll_val = XS1_SS_PLL_CTL_INPUT_DIVISOR_SET(pll_val, pre_div - 1);
    pll_val = XS1_SS_PLL_CTL_FEEDBACK_MUL_SET(pll_val, mul - 1);
    pll_val = XS1_SS_PLL_CTL_POST_DIVISOR_SET(pll_val, post_div - 1);

    pll_val |= 0x80000000; // Disable reset during setting
    pll_val &= 0xbfffffff; // Clock disabled during setting, allowing big VCO jumps
    set_node_pll_reg(tileid, pll_val);
}

void get_node_pll_ratio(unsigned tileid, unsigned *pre_div, unsigned *mul, unsigned *post_div)
{
    unsigned pll_val = get_node_pll_reg(tileid);
    *pre_div = XS1_SS_PLL_CTL_INPUT_DIVISOR(pll_val) + 1;
    *mul = XS1_SS_PLL_CTL_FEEDBACK_MUL(pll_val) + 1;
    *post_div = XS1_SS_PLL_CTL_POST_DIVISOR(pll_val) + 1;
}

unsigned get_core_clock(unsigned tileid)
{
    unsigned pll_val = get_node_pll_reg(tileid);
    return (XTAL_MHZ * (XS1_SS_PLL_CTL_FEEDBACK_MUL(pll_val) + 1) ) / (2 * (XS1_SS_PLL_CTL_INPUT_DIVISOR(pll_val) + 1 ) * (XS1_SS_PLL_CTL_POST_DIVISOR(pll_val) + 1 ));
}

unsigned get_ref_clock(unsigned tileid)
{
    return get_core_clock(tileid) / get_node_ref_clk_div(tileid);
}

unsigned get_switch_clock(unsigned tileid)
{
    return get_core_clock(tileid) / get_node_switch_clk_div(tileid);
}

unsigned get_tile_processor_clock(unsigned tileid)
{
    if (getps(XS1_PS_XCORE_CTRL0) & XS1_XCORE_CTRL0_CLK_DIVIDER_EN_SET(0, 1))
    {
        return get_core_clock(tileid) / get_tile_processor_clk_div(tileid);
    } else {
        return get_core_clock(tileid);
    }
}

unsigned get_tile_vco_clock(unsigned tileid)
{
    unsigned pll_val = get_node_pll_reg(tileid);
    return get_core_clock(tileid) * XS1_SS_PLL_CTL_POST_DIVISOR(pll_val);
}

void disable_tile_processor_clock(unsigned tileid)
{
    unsigned reg_val = 0;
    (void) read_sswitch_reg(tileid, XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, &reg_val);
    (void) write_pswitch_reg(tileid, XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, XS1_PLL_CLK_DISABLE_SET(reg_val, 1));
}

void scale_links(unsigned start_addr, unsigned end_addr, unsigned delay_intra, unsigned delay_inter)
{
    for (int i=start_addr; i<=end_addr; i++)
    {
        unsigned reg_val = 0;
        unsigned tileid = get_local_tile_id();
        (void) read_sswitch_reg(tileid, i, &reg_val);
        reg_val = XS1_XLINK_INTRA_TOKEN_DELAY_SET(reg_val, delay_intra - 1);
        reg_val = XS1_XLINK_INTER_TOKEN_DELAY_SET(reg_val, delay_inter - 1);
        (void) write_sswitch_reg(tileid, i, reg_val);
    }
}

void reset_links(unsigned tileid, unsigned start_addr, unsigned end_addr)
{
    for (int i=start_addr; i<=end_addr; i++)
    {
        unsigned reg_val = 0;
        (void) read_sswitch_reg(tileid, i, &reg_val);
        reg_val = XS1_XLINK_HELLO_SET(reg_val, 1);
        reg_val = XS1_XLINK_RX_RESET_SET(reg_val, 1);
        (void) write_sswitch_reg(tileid, i, reg_val);
    }
}

void dump_links(unsigned start_addr, unsigned end_addr)
{
    for (int i=start_addr; i<=end_addr; i++)
    {
        unsigned reg_val = 0;
        (void) read_sswitch_reg(get_local_tile_id(), i, &reg_val);
        debug_printf("link :0x%x, raw: 0x%8x, intra: %d, inter: %d\n", i, reg_val, XS1_XLINK_INTRA_TOKEN_DELAY(reg_val), XS1_XLINK_INTER_TOKEN_DELAY(reg_val));
    }
}
