// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include <string.h>
#include "soc.h"
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */



static QueueHandle_t xQueueTest0;
static QueueHandle_t xQueueTest1;

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_test0, device, buf, len)
{
    (void) device;
    BaseType_t xYieldRequired = pdFALSE;
    intertile_cb_header_t* tmp = (intertile_cb_header_t*)buf;

    rtos_printf("test0[%d] rx %d header id:%d bytes:%s\n",
            get_core_id(),
            len, tmp->cb_id, buf+1);

    return xYieldRequired;
}

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_test1, device, buf, len)
{
    (void) device;
    BaseType_t xYieldRequired = pdFALSE;
    intertile_cb_header_t* tmp = (intertile_cb_header_t*)buf;

    rtos_printf("test0[%d] rx %d header id:%d bytes:%s\n",
            get_core_id(),
            len, tmp->cb_id, buf+1);

    return xYieldRequired;
}

static void tx_test(void *arg)
{
    soc_peripheral_t dev = arg;

    uint8_t buf[12] = "Hello World\00";
    uint8_t buf1[3] = "Hi\00";

    intertile_cb_header_t test0;
    intertile_driver_header_init(&test0, INTERTILE_CB_ID_1);
    intertile_driver_register_callback( dev, intertile_dev_test0, &test0);

    uint8_t abuf[5] = "test\00";
    intertile_cb_header_t* test1 = pvPortMalloc(sizeof(intertile_cb_header_t)+(5*(sizeof(uint8_t))));
    intertile_driver_header_init(test1, INTERTILE_CB_ID_2);
    intertile_driver_register_callback( dev, intertile_dev_test1, &test1);

    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        rtos_printf("tx_task\n");

        intertile_driver_send_bytes(dev, (uint8_t*)&buf, 12, &test0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        intertile_driver_send_bytes(dev, (uint8_t*)&buf1, 3, test1);

    }
}

typedef struct {
    soc_peripheral_t device;
    QueueHandle_t xQueueTest0;
    QueueHandle_t xQueueTest1;
} rx_test_args_t;

static void rx_test(void *arg)
{
    rx_test_args_t* args = arg;
    soc_peripheral_t dev = args->device;
    QueueHandle_t xQueueTest0 = args->xQueueTest0;
    QueueHandle_t xQueueTest1 = args->xQueueTest1;

    uint8_t buf[8] = "Goodbye\00";

    intertile_cb_header_t test0;
    intertile_driver_header_init(&test0, INTERTILE_CB_ID_1);
    intertile_driver_register_callback( dev, intertile_dev_test0, &test0);

    intertile_cb_header_t test1;
    intertile_driver_header_init(&test1, INTERTILE_CB_ID_2);
    intertile_driver_register_callback( dev, intertile_dev_test1, &test1);

    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        rtos_printf("rx_task\n");

        intertile_driver_send_bytes(dev, (uint8_t*)&buf, 8, &test0);
    }
}


void soc_tile0_main(
        int tile)
{
    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            NULL,
            0);

    xTaskCreate(tx_test, "tx_test", portTASK_STACK_DEPTH(tx_test), dev, 15, NULL);

    vTaskStartScheduler();
}

void soc_tile1_main(
        int tile)
{
    xQueueTest0 = xQueueCreate(2, INTERTILE_DEV_BUFSIZE);
    xQueueTest1 = xQueueCreate(2, INTERTILE_DEV_BUFSIZE);

    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            NULL,
            0);

    rx_test_args_t args = {dev, xQueueTest0, xQueueTest1};

    xTaskCreate(rx_test, "rx_test", portTASK_STACK_DEPTH(rx_test), &args, 15, NULL);

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
