// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"
/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "gpio_driver.h"

/* App headers */
#include "app_conf.h"

#include "xcore_c.h"

void gpio_ctrl_t0(void *arg)
{
    soc_peripheral_t dev = arg;
    uint32_t mabs_buttons;
    uint32_t buttonA, buttonB, buttonC, buttonD;

    gpio_init(dev, port_8C);
    gpio_init(dev, port_1K);
    gpio_init(dev, port_1L);
    gpio_init(dev, port_8D);
    gpio_init(dev, port_1P);

    gpio_init(dev, port_4A);

    for (;;) {
        mabs_buttons = gpio_read(dev, port_4A);
        buttonA = gpio_read_pin(dev, port_4A, 0);
        buttonB = gpio_read_pin(dev, port_4A, 1);
        buttonC = gpio_read_pin(dev, port_4A, 2);
        buttonD = gpio_read_pin(dev, port_4A, 3);

//        debug_printf("read full port: %d\n", mabs_buttons);
//
//        debug_printf("read pins A:%d B:%d C:%d D:%d\n", buttonA, buttonB, buttonC, buttonD);
        vTaskDelay(pdMS_TO_TICKS( 10 ));
        gpio_write_pin(dev, port_8C, 0, buttonA);
        gpio_write_pin(dev, port_8C, 1, buttonA);
        gpio_write_pin(dev, port_8C, 2, buttonB);
        gpio_write_pin(dev, port_8C, 3, buttonB);
        gpio_write_pin(dev, port_8C, 4, buttonC);
        gpio_write_pin(dev, port_8C, 5, buttonC);
        gpio_write_pin(dev, port_8C, 6, buttonD);
        gpio_write_pin(dev, port_8C, 7, buttonD);
        vTaskDelay(pdMS_TO_TICKS( 10 ));
    }
}

void gpio_ctrl_create( UBaseType_t priority )
{
    soc_peripheral_t dev;

    dev = gpio_driver_init(BITSTREAM_GPIO_DEVICE_A);

    xTaskCreate(gpio_ctrl_t0, "t0_gpio_ctrl", portTASK_STACK_DEPTH(gpio_ctrl_t0), dev, priority, NULL);

    dev = gpio_driver_init(BITSTREAM_GPIO_DEVICE_B);

//    xTaskCreate(gpio_ctrl_t1, "t1_gpio_ctrl", portTASK_STACK_DEPTH(gpio_ctrl_t1), dev, priority, NULL);
}
