// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel_streaming.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"
#include "tile_support.h"
#include "platform_init.h"

void gpio_server(chanend_t c_gpio)
{
    port_t p_leds = PORT_LEDS;
    port_t p_btns = PORT_BUTTONS;

    port_enable(p_leds);
    port_enable(p_btns);

    uint32_t led_val = 0;
    uint32_t btn_val = port_in(p_btns);

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(p_btns, event_btn);
    TRIGGERABLE_SETUP_EVENT_VECTOR(c_gpio, event_chan);

    port_set_trigger_in_not_equal(p_btns, btn_val);

    triggerable_enable_trigger(p_btns);
    triggerable_enable_trigger(c_gpio);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(event_btn, event_chan);
        {
            event_btn:
            {
                btn_val = port_in(p_btns);
                port_set_trigger_value(p_btns, btn_val);
                int buttonA = ( btn_val >> 0 ) & 0x01;
                int buttonB = ( btn_val >> 1 ) & 0x01;
                if (buttonA == 0)
                {
                    debug_printf("Button A pressed\n");
                    led_val |= 0x01;
                } else {
                    led_val &= 0xFE;
                }

                if (buttonB == 0)
                {
                    debug_printf("Button B pressed\n");
                    led_val |= 0x02;
                } else {
                    led_val &= 0xFD;
                }
                port_out(p_leds, led_val);
            }
            continue;
        }
        {
            event_chan:
            {
                ;
            }
            continue;
        }
    }
}
