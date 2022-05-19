// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/interrupt_wrappers.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"
#include "burn.h"

DECLARE_INTERRUPT_PERMITTED(void, uart_demo, uart_tx_t* uart);


void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c1;
    (void)c2;
    (void)c3;

    uart_tx_t uart_tx;

    PAR_JOBS (
        PJOB(INTERRUPT_PERMITTED(uart_demo), (&uart_tx))
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ())
    );
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c1;
    (void)c2;
    (void)c3;

}
