// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "usb_support.h"
#include "tusb.h"

#define USB_TASK_STACK_SIZE 1000

void usb_task(void* args)
{
    tusb_init();

    while(1) {
        tud_task();
    }
}

void usb_manager_start(unsigned priority)
{
    xTaskCreate((TaskFunction_t) usb_task,
                "usb_task",
				USB_TASK_STACK_SIZE,
                NULL,
				priority,
                NULL);
}
