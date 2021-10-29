// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/chanend.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"

#define HEARTBEAT_TICKS 50000000

void gpio_server(chanend_t c_from_gpio, chanend_t c_to_gpio)
{
    port_t p_leds = PORT_LEDS;
    port_t p_btns = PORT_BUTTONS;
    hwtimer_t tmr = hwtimer_alloc();

    port_enable(p_leds);
    port_enable(p_btns);

    uint32_t led_val = 0;
    uint32_t heartbeat_val = 0;
    uint32_t btn_val = port_in(p_btns);

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(p_btns, event_btn);
    TRIGGERABLE_SETUP_EVENT_VECTOR(c_to_gpio, event_chan);
    TRIGGERABLE_SETUP_EVENT_VECTOR(tmr, event_timer);

    port_set_trigger_in_not_equal(p_btns, btn_val);
    hwtimer_set_trigger_time(tmr, hwtimer_get_time(tmr) + HEARTBEAT_TICKS);

    triggerable_enable_trigger(p_btns);
    triggerable_enable_trigger(c_to_gpio);
    triggerable_enable_trigger(tmr);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(event_btn, event_chan, event_timer);
        {
            event_btn:
            {
                btn_val = port_in(p_btns);
                port_set_trigger_value(p_btns, btn_val);
                int btn0 = ( btn_val >> 0 ) & 0x01;
                int btn1 = ( btn_val >> 1 ) & 0x01;
                if (btn0 == 0)
                {
                    debug_printf("Button A pressed\n");
                    chanend_out_byte(c_from_gpio, 0x01);
                    led_val |= 0x01;
                } else {
                    led_val &= ~0x01;
                }

                if (btn1 == 0)
                {
                    debug_printf("Button B pressed\n");
                    chanend_out_byte(c_from_gpio, 0x02);
                    led_val |= 0x02;
                } else {
                    led_val &= ~0x02;
                }
                port_out(p_leds, led_val);
            }
            continue;
        }
        {
            event_chan:
            {
                char req_led_val = chanend_in_byte(c_to_gpio);
                if (req_led_val != 0) {
                    led_val |= 0x04;
                } else {
                    led_val &= ~0x04;
                }
                port_out(p_leds, led_val);
            }
            continue;
        }
        {
            event_timer:
            {
                hwtimer_set_trigger_time(tmr, hwtimer_get_time(tmr) + HEARTBEAT_TICKS);
                heartbeat_val ^= 1;
                if (heartbeat_val != 0) {
                    led_val |= 0x08;
                } else {
                    led_val &= ~0x08;
                }
                port_out(p_leds, led_val);
            }
            continue;
        }
    }
}
