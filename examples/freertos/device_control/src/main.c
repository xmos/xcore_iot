// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos_printf.h"
#include "usb_support.h"

/* App headers */
#include "app_control/app_control.h"
#include "device_control.h"
#include "device_control_i2c.h"
#include "device_control_usb.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }
    rtos_printf("Raw bytes to be sent are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control WRITE\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command has %d bytes\n\t", payload_len);
    rtos_printf("Bytes received are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    static device_control_servicer_t servicer_ctx;
    control_ret_t dc_ret;

	for (;;) {
        #if ON_TILE(0)
        {
            control_resid_t resources[] = {0x3, 0x6, 0x9};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
        #if ON_TILE(1)
        {
            control_resid_t resources[] = {0x33, 0x66, 0x99};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
	}
}

static void tile_common_init(chanend_t c)
{
    control_ret_t ctrl_ret;

    platform_init(c);
    chanend_free(c);

    ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);

#if appconfUSB_CTRL_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_init();
    usb_manager_start(appconfUSB_DEVICE_CTRL_TASK_PRIORITY);
#endif

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
