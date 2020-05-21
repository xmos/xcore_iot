// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "gpio_driver.h"

#if ( SOC_GPIO_PERIPHERAL_USED == 0 )
#define BITSTREAM_GPIO_DEVICE_COUNT 0
soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];
#endif /* SOC_GPIO_PERIPHERAL_USED */

#if defined(configNUM_CORES) && configNUM_CORES > 1
#define SMP 1
#else
#define SMP 0
#endif


typedef struct gpio_isr_callback {
    GPIO_ISR_CALLBACK_ATTR gpio_isr_cb_t cb;
} gpio_isr_callback_t;

typedef struct {
    soc_peripheral_t dev;
    int lock; /* A spinlock used when SMP with > 1 core */
    gpio_isr_callback_t gpio_isr_callback_map[ GPIO_TOTAL_PORT_CNT ];
} gpio_driver_t;

RTOS_IRQ_ISR_ATTR
static void gpio_isr( soc_peripheral_t device )
{
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(device);
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    status = soc_peripheral_interrupt_status(device);

    while( status != 0 )
    {
        gpio_id_t source_id = 31UL - ( uint32_t ) __builtin_clz( status );

        xassert( source_id >= 0 );

        status &= ~( 1 << source_id );

        if( driver_struct->gpio_isr_callback_map[source_id].cb != NULL )
        {
            if( driver_struct->gpio_isr_callback_map[source_id].cb(device, source_id) == pdTRUE )
            {
                xYieldRequired = pdTRUE;
            }
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

#if SMP
static int test_and_set(int *var, int val)
{
    int ret;
    portGET_ISR_LOCK();
    {
        ret = *var;
        *var = val;
    }
    portRELEASE_ISR_LOCK();

    return ret;
}
#endif

/*
 * The lock function always disables interrupts to
 * protect against simultaneous channel access from
 * an ISR on the same core. When there is more than
 * one RTOS core, a spinlock is also acquired to
 * protect against simultaneous channel access from
 * both tasks and ISRs running on other cores.
 *
 * An RTOS mutex could have been used here if ISRs
 * were not allowed to use the GPIO driver functions,
 * but it feels important that ISRs be able to at
 * least read and write to GPIO.
 */
static void gpio_lock_get(int *lock)
{
    taskDISABLE_INTERRUPTS();
#if SMP
    while (test_and_set(lock, 1));
#else
    (void) lock;
#endif
}

static void gpio_lock_release(int *lock)
{
#if SMP
    *lock = 0;
#else
    (void) lock;
#endif
    if (portCHECK_IF_IN_ISR() == pdFALSE) {
        taskENABLE_INTERRUPTS();
    }
}

static int gpio_driver_alloc(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_ALLOC );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_free(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_FREE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_read(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t *data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IN );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(uint32_t), data,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_write(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_OUT );

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(id), &id,
            sizeof(data), &data);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_peek(
        soc_peripheral_t dev,
        gpio_id_t id,
        uint32_t *data)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_PEEK );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 2,
            sizeof(uint32_t), data,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_irq_setup(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_SETUP );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_irq_enable(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_ENABLE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

static int gpio_driver_irq_disable(
        soc_peripheral_t dev,
        gpio_id_t id)
{
    int retval;
    chanend c = soc_peripheral_ctrl_chanend( dev );
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);

    gpio_lock_get(&driver_struct->lock);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_IRQ_DISABLE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(int), &retval);

    gpio_lock_release(&driver_struct->lock);

    return retval;
}

int gpio_irq_setup_callback( soc_peripheral_t dev, gpio_id_t gpio_id, gpio_isr_cb_t isr_cb)
{
    gpio_driver_t *driver_struct = (gpio_driver_t *) soc_peripheral_app_data(dev);
    uint32_t retVal;

    driver_struct->gpio_isr_callback_map[gpio_id].cb = isr_cb;

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
        int isr_core)
{
    soc_peripheral_t device;
    gpio_driver_t *driver_struct;

    xassert( device_id >= 0 && device_id < BITSTREAM_GPIO_DEVICE_COUNT );

    device = bitstream_gpio_devices[ device_id ];

    driver_struct = pvPortMalloc(sizeof(gpio_driver_t));

    driver_struct->lock = 0;
    memset(driver_struct->gpio_isr_callback_map, 0, sizeof(driver_struct->gpio_isr_callback_map));
    driver_struct->dev = device;

    soc_peripheral_handler_register( device, isr_core, driver_struct, (rtos_irq_isr_t)gpio_isr );

    return device;
}
