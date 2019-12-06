// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "gpio_driver.h"

/* App headers */
#include "app_conf.h"

#include "xcore_c.h"

static QueueHandle_t gpio_event_q;
static TaskHandle_t gpio_handler_task;

RTOS_IRQ_ISR_ATTR
int gpio_isr( soc_peripheral_t device )
{
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    configASSERT(device == bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A]);

    status = soc_peripheral_interrupt_status(device);
    xTaskNotifyFromISR( gpio_handler_task, status, eSetBits, &xYieldRequired );

    return xYieldRequired;
}

void gpio_ctrl_t0(void *arg)
{
    soc_peripheral_t dev = arg;
    uint32_t mabs_buttons;
    uint32_t buttonA, buttonB, buttonC, buttonD;
    uint32_t status;

    /* Initialize LED outputs */
    gpio_init(dev, gpio_8C);
    gpio_init(dev, gpio_1K);
    gpio_init(dev, gpio_1L);
    gpio_init(dev, gpio_8D);
    gpio_init(dev, gpio_1P);

    /* Initialize button inputs */
    gpio_init(dev, gpio_4A);

    /* Enable interrupts on buttons */
    gpio_irq_setup(dev, gpio_4A);
    gpio_irq_enable(dev, gpio_4A);

    for (;;) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                &status,         /* Pass out notification value into status */
                portMAX_DELAY ); /* Wait indefinitely until next notification */

        if( ( status & gpio_4A ) != 0 )
        {
            mabs_buttons = gpio_read(dev, gpio_4A);
            buttonA = ( mabs_buttons >> 0 ) & 0x01;
            buttonB = ( mabs_buttons >> 1 ) & 0x01;
            buttonC = ( mabs_buttons >> 2 ) & 0x01;
            buttonD = ( mabs_buttons >> 3 ) & 0x01;
        }

        gpio_write_pin(dev, gpio_8C, 0, buttonA);
        gpio_write_pin(dev, gpio_8C, 1, buttonA);
        gpio_write_pin(dev, gpio_8C, 2, buttonB);
        gpio_write_pin(dev, gpio_8C, 3, buttonB);
        gpio_write_pin(dev, gpio_8C, 4, buttonC);
        gpio_write_pin(dev, gpio_8C, 5, buttonC);
        gpio_write_pin(dev, gpio_8C, 6, buttonD);
        gpio_write_pin(dev, gpio_8C, 7, buttonD);
    }
}

void gpio_ctrl_create( UBaseType_t priority )
{
    soc_peripheral_t dev;

    gpio_event_q = xQueueCreate(2, sizeof(void *));

    dev = gpio_driver_init(
            BITSTREAM_GPIO_DEVICE_A,        /* Initializing GPIO device A */
            NULL,                           /* No app data */
            0,                              /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) gpio_isr );    /* The ISR to handle this device's interrupts */

    xTaskCreate(gpio_ctrl_t0, "t0_gpio_ctrl", portTASK_STACK_DEPTH(gpio_ctrl_t0), dev, priority, &gpio_handler_task);

//    dev = gpio_driver_init(BITSTREAM_GPIO_DEVICE_B);

//    xTaskCreate(gpio_ctrl_t1, "t1_gpio_ctrl", portTASK_STACK_DEPTH(gpio_ctrl_t1), dev, priority, NULL);
}
