// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef GPIO_DRIVER_H_
#define GPIO_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "gpio_dev_ctrl.h"
#include "gpio_port_map.h"

/* Initialize device */
soc_peripheral_t gpio_driver_init(
        int device_id,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

/* Initialize port */
int gpio_init( soc_peripheral_t dev, gpio_id_t gpio_id );

/* Write to port */
void gpio_write( soc_peripheral_t dev, gpio_id_t gpio_id, uint32_t value );
void gpio_write_pin( soc_peripheral_t dev, gpio_id_t gpio_id, int pin, uint32_t value );

/* Read from port */
uint32_t gpio_read( soc_peripheral_t dev, gpio_id_t gpio_id );
uint32_t gpio_read_pin( soc_peripheral_t dev, gpio_id_t gpio_id, int pin );

/* Free port */
void gpio_free( soc_peripheral_t dev, gpio_id_t gpio_id );

/* GPIO IRQ support */
int gpio_irq_setup( soc_peripheral_t dev, gpio_id_t gpio_id );
int gpio_irq_enable( soc_peripheral_t dev, gpio_id_t gpio_id );
int gpio_irq_disable( soc_peripheral_t dev, gpio_id_t gpio_id );

#endif /* GPIO_DRIVER_H_ */
