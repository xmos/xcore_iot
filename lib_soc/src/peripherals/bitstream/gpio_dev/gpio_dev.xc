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
void gpio_handler(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ctrl_c)
{
    uint32_t cmd;
    unsigned port_ptr;
    int port_id;

    unsigned port_arg;
    uint32_t data;

    while (1) {
        select
        {
        case !isnull(ctrl_c) => xcore_freertos_periph_function_code_rx(ctrl_c, &cmd):
            switch( cmd )
            {
            case GPIO_DEV_PORT_ALLOC:
                xcore_freertos_periph_varlist_rx(
                        ctrl_c, 2,
                        sizeof(port_ptr), &port_ptr,
                        sizeof(port_id), &port_id);

                gpio_port_alloc(&port_ptr, port_id);

                xcore_freertos_periph_varlist_tx(
                        ctrl_c, 1,
                        sizeof(port_ptr), &port_ptr);
                break;

            case GPIO_DEV_PORT_FREE:
                xcore_freertos_periph_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_ptr), &port_ptr);

                gpio_port_free(&port_ptr);
                break;

            case GPIO_DEV_PORT_PEEK:
                xcore_freertos_periph_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_arg), &port_arg);

                gpio_port_peek(port_arg, &data);

                xcore_freertos_periph_varlist_tx(
                        ctrl_c, 1,
                        sizeof(data), &data);
                break;

            case GPIO_DEV_PORT_IN:
                xcore_freertos_periph_varlist_rx(
                        ctrl_c, 1,
                        sizeof(port_arg), &port_arg);

                gpio_port_in(port_arg, &data);

                xcore_freertos_periph_varlist_tx(
                        ctrl_c, 1,
                        sizeof(data), &data);
                break;

            case GPIO_DEV_PORT_OUT:
                xcore_freertos_periph_varlist_rx(
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

void gpio_dev(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ctrl_c)
{
    par
    {
        gpio_handler(
              data_to_dma_c,
              data_from_dma_c,
              ctrl_c
                );
    }
}













