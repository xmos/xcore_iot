// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_CLOCK_CONTROL_H_
#define XCORE_CLOCK_CONTROL_H_

/**
 * \addtogroup hil_clock_control hil_clock_control
 *
 * The public API for using the clock control HIL library.
 * @{
 */

/**
 * This macro should be overridden via compile definitions
 * to specify the crystal frequency used.
 */
#ifndef XTAL_MHZ
#define XTAL_MHZ 24
#endif

#ifndef __XC__
#include <xs1.h>

/**
 * Declaration to access the core ids in C
 */
extern const unsigned short __core_ids[];

/**
 * Helper macro to access tile id by tile number
 */
#define TILE_ID(num)    (unsigned)__core_ids[num]
#endif /* __XC__ */

/**
 * Enables the clock divider register on the caller's tile
 */
void enable_local_tile_processor_clock_divider(void);

/**
 * Sets the reference clock divider register value on a specified tile
 *
 * Note: It is up to the application to ensure that it is safe to change
 * the clock on another tile.
 *
 * \param tileid   The tileid of the tile to change the reference clock divider
 *                 value.  Using the TILE_ID() macro is recommended.
 * \param divider  The value + 1 to write to XS1_SSWITCH_REF_CLK_DIVIDER_NUM
 */
void set_node_ref_clk_div(unsigned tileid, unsigned divider);

/**
 * Helper macro to call set_node_ref_clk_div() for the caller's tile
 */
#define set_local_node_ref_clk_div(divider)  set_node_ref_clk_div(get_local_tile_id(), divider)

/**
 * Gets the reference clock divider register value on a specified tile
 *
 * \param tileid   The tileid of the tile to get the reference clock divider
 *                 value.  Using the TILE_ID() macro is recommended.
 *
 * \returns        The value of XS1_SSWITCH_REF_CLK_DIVIDER_NUM + 1
 */
unsigned get_node_ref_clk_div(unsigned tileid);

/**
 * Helper macro to call get_node_ref_clk_div() for the caller's tile
 */
#define get_local_node_ref_clk_div()  get_node_ref_clk_div(get_local_tile_id())

/**
 * Sets the system switch clock divider register value on a specified tile
 *
 * Note: It is up to the application to ensure that it is safe to change
 * the clock on another tile.
 *
 * \param tileid   The tileid of the tile to change the system switch clock
 *                 divider value.  Using the TILE_ID() macro is recommended.
 * \param divider  The value + 1 to write to XS1_SSWITCH_CLK_DIVIDER_NUM
 */
void set_node_switch_clk_div(unsigned tileid, unsigned divider);

/**
 * Helper macro to call set_node_switch_clk_div() for the caller's tile
 */
#define set_local_node_switch_clk_div(divider)   set_node_switch_clk_div(get_local_tile_id(), divider)

/**
 * Gets the system switch clock divider register value on a specified tile
 *
 * \param tileid   The tileid of the tile to get the system switch clock
 *                 divider value.  Using the TILE_ID() macro is recommended.
 *
 * \returns        The value of XS1_SSWITCH_CLK_DIVIDER_NUM + 1
 */
unsigned get_node_switch_clk_div(unsigned tileid);

/**
 * Helper macro to call get_node_switch_clk_div() for the caller's tile
 */
#define get_local_node_switch_clk_div()   get_node_switch_clk_div(get_local_tile_id())

/**
 * Sets the tile clock divider register value on a specified tile
 *
 * Note: This will only work if enable_local_tile_processor_clock_divider() has
 * been called prior on the caller tile.
 *
 * Note: It is up to the application to ensure that it is safe to change
 * the clock on another tile.
 *
 * \param tileid   The tileid of the tile to change the tile clock
 *                 divider value.  Using the TILE_ID() macro is recommended.
 * \param divider  The value + 1 to write to XS1_PSWITCH_PLL_CLK_DIVIDER_NUM
 */
void set_tile_processor_clk_div(unsigned tileid, unsigned divider);

/**
 * Helper macro to call set_tile_processor_clk_div() for the caller's tile
 */
#define set_local_tile_processor_clk_div(divider)   set_tile_processor_clk_div(get_local_tile_id(), divider)

/**
 * Gets the tile clock divider register value on a specified tile
 *
 * \param tileid   The tileid of the tile to get the tile clock divider
 *                 value.  Using the TILE_ID() macro is recommended.
 *
 * \returns        The value of XS1_PSWITCH_PLL_CLK_DIVIDER_NUM + 1
 */
unsigned get_tile_processor_clk_div(unsigned tileid);

/**
 * Helper macro to call get_tile_processor_clk_div() for the caller's tile
 */
#define get_local_tile_processor_clk_div()   get_tile_processor_clk_div(get_local_tile_id())

/**
 * Sets the tile clock PLL control register value on a specified tile.
 *
 * Note: It is up to the application to ensure that it is safe to change
 * the clock.
 *
 * \param tileid   The tileid of the tile to change the PLL control register
 *                 value for.  Using the TILE_ID() macro is recommended.
 * \param divider  The value write to XS1_SSWITCH_PLL_CTL_NUM
 */
void set_node_pll_reg(unsigned tileid, unsigned reg_val);

/**
 * Helper macro to call set_node_pll_reg() for the caller's tile
 */
#define set_local_node_pll_reg(reg_val)   set_node_pll_reg(get_local_tile_id(), reg_val)

/**
 * Gets the tile clock PLL control register value on a specified tile.
 *
 * \param tileid   The tileid of the tile to get the PLL control register
 *                 value for.  Using the TILE_ID() macro is recommended.
 *
 * \returns        The value of XS1_SSWITCH_PLL_CTL_NUM
 */
unsigned get_node_pll_reg(unsigned tileid);

/**
 * Helper macro to call get_node_pll_reg() for the caller's tile
 */
#define get_local_pll_reg()   get_node_pll_reg(get_local_tile_id())

/**
 * Sets the tile clock PLL control register value on a specified tile.  The
 * value set is calculated from the divider stage 1, multiplier stage, and
 * divider stage 2 values provided.
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
 * \param tileid   The tileid of the tile to change the PLL control register
 *                 values for.  Using the TILE_ID() macro is recommended.
 * \param pre_div  The value of R
 * \param mul      The value of F
 * \param post_div The value of OD
 */
void set_node_pll_ratio(unsigned tileid, unsigned pre_div, unsigned mul, unsigned post_div);

/**
 * Helper macro to call set_node_pll_ratio() for the caller's tile
 */
 #define set_local_node_pll_ratio(pre_div, mul, post_div)   set_node_pll_ratio(get_local_tile_id(), pre_div, mul, post_div)

/**
 * Gets the divider stage 1, multiplier stage, and divider stage 2 values
 * from the tile clock PLL control register values of a specified tile.
 *
 * \param tileid   The tileid of the tile to get the PLL control register
 *                 values for.  Using the TILE_ID() macro is recommended.
 * \param pre_div  A pointer to be populated with the value of R
 * \param mul      A pointer to be populated with the value of F
 * \param post_div A pointer to be populated with the value of OD
 */
void get_node_pll_ratio(unsigned tileid, unsigned *pre_div, unsigned *mul, unsigned *post_div);

/**
 * Helper macro to call get_node_pll_ratio() for the caller's tile
 */
#define get_local_node_pll_ratio(pre_div, mul, post_div)   get_node_pll_ratio(get_local_tile_id(), pre_div, mul, post_div)

/**
 * Gets the calculated core clock frequency.
 *
 * Note: To be accurate, the symbol XTAL_MHZ must be defined with the value
 * appropriate oscillator frequency being used.
 *
 * \param tileid   The tileid of the tile to get the clock value for.
 *                 Using the TILE_ID() macro is recommended.
 *
 * \returns        The calculated core clock frequency
 */
unsigned get_core_clock(unsigned tileid);

/**
 * Helper macro to call get_core_clock() for the caller's tile
 */
#define get_local_core_clock()   get_core_clock(get_local_tile_id())

/**
 * Gets the calculated reference clock frequency.
 *
 * Note: To be accurate, the symbol XTAL_MHZ must be defined with the value
 * appropriate oscillator frequency being used.
 *
 * \param tileid   The tileid of the tile to get the reference clock value for.
 *                 Using the TILE_ID() macro is recommended.
 *
 * \returns        The calculated reference clock frequency
 */
unsigned get_ref_clock(unsigned tileid);

/**
 * Helper macro to call get_ref_clock() for the caller's tile
 */
#define get_local_ref_clock()   get_ref_clock(get_local_tile_id())

/**
 * Gets the calculated switch clock frequency.
 *
 * Note: To be accurate, the symbol XTAL_MHZ must be defined with the value
 * appropriate oscillator frequency being used.
 *
 * \param tileid   The tileid of the tile to get the switch clock value for.
 *                 Using the TILE_ID() macro is recommended.
 *
 * \returns        The calculated switch clock frequency
 */
unsigned get_switch_clock(unsigned tileid);

/**
 * Helper macro to call get_switch_clock() for the caller's tile
 */
#define get_local_switch_clock()   get_switch_clock(get_local_tile_id())

/**
 * Gets the core clock frequency.
 *
 * Note: To be accurate, the symbol XTAL_MHZ must be defined with the value
 * appropriate oscillator frequency being used.
 *
 * \param tileid   The tileid of the tile to get the core clock value for.
 *                 Using the TILE_ID() macro is recommended.
 *
 * \returns        The core clock frequency
 */
unsigned get_tile_processor_clock(unsigned tileid);

/**
 * Helper macro to call get_tile_processor_clock() for the caller's tile
 */
#define get_local_tile_processor_clock()   get_tile_processor_clock(get_local_tile_id())

/**
 * Gets the calculated voltage-controlled oscillator (VCO) clock frequency.
 *
 * Note: To be accurate, the symbol XTAL_MHZ must be defined with the value
 * appropriate oscillator frequency being used.
 *
 * \param tileid   The tileid of the tile to get the VCO clock value for.
 *                 Using the TILE_ID() macro is recommended.
 *
 * \returns        The calculated VCO clock frequency
 */
unsigned get_tile_vco_clock(unsigned tileid);

/**
 * Helper macro to call get_tile_vco_clock() for the caller's tile
 */
#define get_local_tile_vco_clock()   get_tile_vco_clock(get_local_tile_id())

/**
 * Disable the tile pll clock
 *
 * Note: This cannot be undone and recovery requires a reset
 *
 * \param tileid   The tileid of the tile to get the pll clock value for.
 *                 Using the TILE_ID() macro is recommended.
 */
void disable_tile_processor_clock(unsigned tileid);

/**
 * Helper macro to call disable_tile_processor_clock() for the caller's tile
 */
#define disable_local_tile_processor_clock()   disable_tile_processor_clock(get_local_tile_id())

/**
 * Sets the intra token delay and inter token delay to the xlinks within an
 * address range, inclusive.
 *
 * \param start_addr   The starting link address
 * \param end_addr     The ending address
 * \param delay_intra  The intra token delay value
 * \param delay_inter  The inter token delay value
 */
void scale_links(unsigned start_addr, unsigned end_addr, unsigned delay_intra, unsigned delay_inter);

/**
 * Resets the xlinks within an address range, inclusive
 *
 * \param tileid       The tileid of the tile to reset links on
 * \param start_addr   The starting link address
 * \param end_addr     The ending address
 */
void reset_links(unsigned tileid, unsigned start_addr, unsigned end_addr);

/**
 * Helper macro to call reset_links() for the caller's tile
 */
#define reset_local_links(start_addr, end_addr)   reset_links(get_local_tile_id(), start_addr, end_addr)

/**
 * Debug function that prints info on the xlinks within a specified address
 * range inclusive.
 *
 * \param start_addr   The starting link address
 * \param end_addr     The ending link address
 */
void dump_links(unsigned start_addr, unsigned end_addr);

/**@}*/ // END: addtogroup hil_clock_control

#endif /* XCORE_CLOCK_CONTROL_H_ */
