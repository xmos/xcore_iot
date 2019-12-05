// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>
#include <xs1.h>

#include "soc.h"
#include "xassert.h"
#include "gpio_dev.h"

#include "debug_print.h"

unsafe
{
[[combinable]]
void gpio_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        chanend ?irq_c)
{
    uint32_t cmd;
    unsigned port_ptr;
    int port_id;

    unsigned port_arg;
    uint32_t data;

    while (1) {
        select
        {
        case !isnull(ctrl_c) => soc_peripheral_function_code_rx(ctrl_c, &cmd):
            switch( cmd )
            {
            case GPIO_DEV_PORT_ALLOC:
                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(port_ptr), &port_ptr,
                        sizeof(port_id), &port_id);

                gpio_port_alloc(&port_ptr, port_id);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(port_ptr), &port_ptr);
                break;

            case GPIO_DEV_PORT_FREE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_ptr), &port_ptr);

                gpio_port_free(&port_ptr);
                break;

            case GPIO_DEV_PORT_PEEK:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_arg), &port_arg);

                gpio_port_peek(port_arg, &data);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(data), &data);
                break;

            case GPIO_DEV_PORT_IN:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_arg), &port_arg);

                gpio_port_in(port_arg, &data);

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(data), &data);
                break;

            case GPIO_DEV_PORT_OUT:
                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(port_arg), &port_arg,
                        sizeof(data), &data);

                gpio_port_out(port_arg, data);
                break;

            default:
                fail("Invalid CMD");
                break;
            }
            break;
        }
    }
}
}
