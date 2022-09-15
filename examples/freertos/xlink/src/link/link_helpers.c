// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xs3a_registers.h>

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "link_helpers.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#ifdef appconfXLINK_WIRE_TYPE
#define W appconfXLINK_WIRE_TYPE
#else
/* default to 2 wire */
#define W 0
#endif

/* Disable link by resetting ENABLE bit in link's control register */
void link_disable(unsigned tileid, unsigned link_num) {
    unsigned x = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, &x);
    x &= ~XS1_XLINK_ENABLE_MASK;
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, x);
}

/* Configure link by performing the following in the link's control register
 *   - Set intertoken and intratoken delays
 *   - Set ENABLE bit
 *   - Set WIDE bit if 5-bit link required
 */
void link_enable(unsigned tileid, unsigned link_num) {
    unsigned x = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, &x);
    x |= XS1_XLINK_INTER_TOKEN_DELAY_SET(x, appconfINTER_DELAY);
    x |= XS1_XLINK_INTRA_TOKEN_DELAY_SET(x, appconfINTRA_DELAY);
    x |= XS1_XLINK_ENABLE_MASK;
    x |= W * XS1_XLINK_WIDE_MASK;
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, x);
}

/* Reset link by setting RESET bit in link's control register */
void link_reset(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  x |= XS1_XLINK_RX_RESET_MASK;
  (void) write_sswitch_reg(tileid, l, x);
}

/* Send a HELLO by setting HELLO bit in link's control register */
void link_hello(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  x |= XS1_XLINK_HELLO_MASK;
  (void) write_sswitch_reg(tileid, l, x);
}

unsigned link_got_credit(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  return XS1_TX_CREDIT(x);
}