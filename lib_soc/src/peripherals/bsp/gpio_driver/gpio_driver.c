// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bsp/common/soc_bsp_common.h"
#include "bitstream_devices.h"

#include "gpio_driver.h"

#if ( SOC_GPIO_PERIPHERAL_USED == 0 )
#define BITSTREAM_GPIO_DEVICE_COUNT 0
soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];
#endif /* SOC_GPIO_PERIPHERAL_USED */

static int gpio_driver_alloc(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_ALLOC );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_free(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_FREE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_read(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t *data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IN );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(uint32_t), data,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_write(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_OUT );

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(id), &id,
            sizeof(data), &data);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_peek(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t *data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_PEEK );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(uint32_t), data,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_irq_setup(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_SETUP );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_irq_enable(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_ENABLE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

static int gpio_driver_irq_disable(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_DISABLE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    return retval;
}

int gpio_irq_setup( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    uint32_t retVal;

    retVal = gpio_driver_irq_setup( dev, gpio_id );

    return retVal;
}

int gpio_irq_enable( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    uint32_t retVal;

    retVal = gpio_driver_irq_enable( dev, gpio_id );

    return retVal;
}

int gpio_irq_disable( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    uint32_t retVal;

    retVal = gpio_driver_irq_disable( dev, gpio_id );

    return retVal;
}

int gpio_init( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    int retVal;

    retVal = gpio_driver_alloc( dev, gpio_id );

    return retVal;
}

void gpio_free( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    gpio_driver_free( dev, gpio_id );
}

void gpio_write( soc_peripheral_t dev, gpio_id_t gpio_id, uint32_t value )
{
    gpio_driver_write( dev, gpio_id, value );
}

void gpio_write_pin( soc_peripheral_t dev, gpio_id_t gpio_id, int pin, uint32_t value )
{
    uint32_t port_state;

    gpio_driver_peek( dev, gpio_id, &port_state );

    if( value == 0 )
    {
        port_state &= ~( 1 << pin );
    }
    else
    {
        port_state |= ( 1 << pin );
    }

    gpio_driver_write( dev, gpio_id, port_state );
}

uint32_t gpio_read( soc_peripheral_t dev, gpio_id_t gpio_id )
{
    uint32_t retVal;

    gpio_driver_read( dev, gpio_id, &retVal );

    return retVal;
}

uint32_t gpio_read_pin( soc_peripheral_t dev, gpio_id_t gpio_id, int pin )
{
    uint32_t retVal;

    gpio_driver_read( dev, gpio_id, &retVal );
    retVal = (( retVal >> pin ) & 0x0001 );

    return retVal;
}

soc_peripheral_t gpio_driver_init(
        int device_id,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_peripheral_t device;

    xassert( device_id >= 0 && device_id < BITSTREAM_GPIO_DEVICE_COUNT );

    device = bitstream_gpio_devices[ device_id ];

    soc_peripheral_handler_register( device, isr_core, app_data, isr );

    return device;
}
