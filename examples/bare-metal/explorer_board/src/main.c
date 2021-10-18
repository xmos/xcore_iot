// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/chanend.h>
#include "soc.h"

// /* App headers */
// #include "app_conf.h"
// #include "platform/platform_init.h"
// #include "platform/driver_instances.h"
// #include "example_pipeline/example_pipeline.h"
// #include "gpio_ctrl/gpio_ctrl.h"

    // platform_start();


static void tile_common_init(chanend_t c)
{
    // platform_init(c);
    chanend_free(c);
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c0;
    (void)c2;
    (void)c3;

    // tile_common_init(c1);
    uint32_t test = 0xDEADBEEF;

    chanend_t c_msg = soc_channel_establish(c1, soc_channel_inout);

    chanend_out_word(c_msg, test);
    chanend_out_control_token(c_msg, XS1_CT_PAUSE);

    while(1) {;}
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c1;
    (void)c2;
    (void)c3;

    // tile_common_init(c0);

    chanend_t c_msg = soc_channel_establish(c0, soc_channel_inout);

    uint32_t test = chanend_in_word(c_msg);

    printf("tile 1 got 0x%x\n", test);

    while(1) {;}
}
