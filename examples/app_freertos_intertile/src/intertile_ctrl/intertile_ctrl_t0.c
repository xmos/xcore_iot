// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_ctrl.h"


void t0_test(void *arg)
{
    soc_peripheral_t dev = arg;

    uint8_t buf[] = "Hello World";
    uint8_t buf1[] = "Hi";

    intertile_cb_header_t test0;
    intertile_driver_header_init(&test0, INTERTILE_CB_ID_1);
    intertile_driver_register_callback( dev, intertile_dev_test0, &test0);

    intertile_cb_header_t* test1 = pvPortMalloc(sizeof(intertile_cb_header_t)+(5*(sizeof(uint8_t))));
    intertile_driver_header_init(test1, INTERTILE_CB_ID_2);
    intertile_driver_register_callback( dev, intertile_dev_test0, test1);

    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        intertile_driver_send_bytes(dev, buf, strlen((char *)buf) + 1, &test0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        intertile_driver_send_bytes(dev, buf1, strlen((char *)buf1) + 1, test1);
    }
}
