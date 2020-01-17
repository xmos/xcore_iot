// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef GPIO_DRIVER_H_
#define GPIO_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "gpio_dev_ctrl.h"
#include "gpio_port_map.h"

#include "FreeRTOS.h"

/* Attribute for the gpio_isr_callback_map member of the gpio isr.
Required by xcc to calculate stack usage. */
#define GPIO_ISR_CALLBACK_ATTR __attribute__((fptrgroup("gpio_isr_callback")))

/* GPIO ISR callback function macros. For xcc this ensures they get added to the gpio isr callback
group so that stack usage for certain functions can be calculated. */
#define GPIO_ISR_CALLBACK_FUNCTION_PROTO( xFunction, device, source_id ) BaseType_t xFunction( soc_peripheral_t device, gpio_id_t source_id )
#define GPIO_ISR_CALLBACK_FUNCTION( xFunction, device, source_id ) GPIO_ISR_CALLBACK_ATTR BaseType_t xFunction( soc_peripheral_t device, gpio_id_t source_id )

typedef BaseType_t (*gpio_isr_cb_t)(soc_peripheral_t device, gpio_id_t source_id);

/* Initialize device */
soc_peripheral_t gpio_driver_init(
        int device_id,
        void *app_data,
        int isr_core);

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
int gpio_irq_setup_callback( soc_peripheral_t dev, gpio_id_t gpio_id, gpio_isr_cb_t isr_callback);
int gpio_irq_enable( soc_peripheral_t dev, gpio_id_t gpio_id );
int gpio_irq_disable( soc_peripheral_t dev, gpio_id_t gpio_id );

#endif /* GPIO_DRIVER_H_ */
