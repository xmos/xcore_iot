// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef GPIO_DEV_H_
#define GPIO_DEV_H_

#define GPIODEV_PORT_NONE               0
#define GPIODEV_PORT_IN                 1
#define GPIODEV_PORT_IN_WITH_EVENTS     2
#define GPIODEV_PORT_OUT                3

#if __rtos_peripherals_conf_h_exists__
#include "rtos_peripherals_conf.h"
#endif

#include "gpio_dev_conf_defaults.h"
#include "gpio_dev_ctrl.h"

#include "gpio.h"

#if (__XC__)
[[combinable]]
void gpio_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        chanend ?irq_c);

extern "C" {
#endif
#include "xccompat.h"
void gpio_port_alloc( unsigned *p, int id );
void gpio_port_free( unsigned *p );

void gpio_port_peek( unsigned p, uint32_t *data );
void gpio_port_in( unsigned p, uint32_t *data );
void gpio_port_out( unsigned p, uint32_t data );
#ifdef __XC__
}
#endif

#endif /* GPIO_DEV_H_ */
