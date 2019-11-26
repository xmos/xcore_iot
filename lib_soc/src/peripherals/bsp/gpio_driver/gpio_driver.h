// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef GPIO_DRIVER_H_
#define GPIO_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "gpio_dev_ctrl.h"

/* Initialize device */
soc_peripheral_t gpio_driver_init(
        int device_id);

/* Initialize port */
int gpio_init( soc_peripheral_t dev, port_id_t p );

/* Write to port */
void gpio_write( soc_peripheral_t dev, port_id_t p, uint32_t value );
void gpio_write_pin( soc_peripheral_t dev, port_id_t p, int pin, uint32_t value );

/* Read from port */
uint32_t gpio_read( soc_peripheral_t dev, port_id_t p );
uint32_t gpio_read_pin( soc_peripheral_t dev, port_id_t p, int pin );

/* Free port */
void gpio_free( soc_peripheral_t dev, port_id_t p );

#endif /* GPIO_DRIVER_H_ */
