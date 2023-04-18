// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>
#include "xcore/parallel.h"

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

#include "xud_device.h"
#include "rtos_usb.h"
#if RUN_EP0_VIA_PROXY
#include "rtos_ep0_proxy.h"
#endif

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

#if RUN_EP0_VIA_PROXY
void startup_task(void *arg)
{
    platform_start();

    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

#if appconfUSB_CTRL_ENABLED
#if ON_TILE(EP0_TILE_NO)
    printf("Call usb_manager_start() on tile %d\n", THIS_XCORE_TILE);
    usb_manager_start(appconfUSB_DEVICE_CTRL_TASK_PRIORITY);
    /* Sync with the other tile */
    int dummy = 0;
    rtos_intertile_tx(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, &dummy, sizeof(dummy));
#else
    printf("Call usb_manager_start() not on tile %d\n", THIS_XCORE_TILE);
    
    ep0_proxy_start(appconfUSB_DEVICE_CTRL_TASK_PRIORITY);

    int ret = 0;
    rtos_intertile_rx_len(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
    rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
#endif
#endif

    static device_control_servicer_t servicer_ctx;
    control_ret_t dc_ret;

	for (;;) {
        #if ON_TILE(0)
        {
            control_resid_t resources[] = {0x3};

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
            control_resid_t resources[] = {0x33};

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

static void tile_common_init_tile1(chanend_t c)
{
    control_ret_t ctrl_ret;

    // Open new cross tile paths tile1 c_ep0_proxy <-> tile 0 c_ep0_proxy using existing channel c0 <-> c1
    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chan_out_word(c, c_ep0_proxy);
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chan_out_word(c, c_ep0_proxy_xfer_complete);
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));

    //printf("Calling tile_common_init_tile1() on tile %d, c_ep0_proxy = %ld\n", THIS_XCORE_TILE, c_ep0_proxy);
    platform_init(c);

    chanend_free(c);

    ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);

#if appconfUSB_CTRL_ENABLED
    usb_manager_init(c_ep0_proxy, c_ep0_proxy_xfer_complete);
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

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init_tile1(c0);
}
#endif

volatile uint32_t noEpOut = 0;
volatile uint32_t noEpIn = 0;
volatile XUD_EpType epTypeTableOut[RTOS_USB_ENDPOINT_COUNT_MAX];
volatile XUD_EpType epTypeTableIn[RTOS_USB_ENDPOINT_COUNT_MAX];
volatile channel_t channel_ep_out[RTOS_USB_ENDPOINT_COUNT_MAX];
volatile channel_t channel_ep_in[RTOS_USB_ENDPOINT_COUNT_MAX];

DECLARE_JOB(_XUD_Main, (chanend_t, chanend_t, chanend_t, XUD_BusSpeed_t, XUD_PwrConfig));
DECLARE_JOB(tile_common_init_tile0, (chanend_t, chanend_t, chanend_t));

void tile_common_init_tile0(chanend_t c, chanend_t c_ep0_out, chanend_t c_ep0_in)
{
    control_ret_t ctrl_ret;

    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy);

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy_xfer_complete);

    platform_init(c);
    chanend_free(c);

    ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);

#if appconfUSB_CTRL_ENABLED
    ep0_proxy_init(c_ep0_out, c_ep0_in, c_ep0_proxy, c_ep0_proxy_xfer_complete);
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

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    // Allocate EP0 endpoint channels
    channel_t channel_ep0_out;
    channel_t channel_ep0_in;

    channel_ep0_out = chan_alloc();
    channel_ep0_in = chan_alloc();

    printf("Calling tile_common_init_tile0() on tile %d\n", THIS_XCORE_TILE);

    PAR_JOBS(        
        PJOB(_XUD_Main, (channel_ep0_out.end_a, channel_ep0_in.end_a, 0, XUD_SPEED_HS, XUD_PWR_BUS)),
        PJOB(tile_common_init_tile0, (c1, channel_ep0_out.end_b, channel_ep0_in.end_b))

    );
}
#endif

#else // #if RUN_EP0_VIA_PROXY

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if appconfUSB_CTRL_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_start(appconfUSB_DEVICE_CTRL_TASK_PRIORITY);
    /* Sync with the other tile */
    int dummy = 0;
    rtos_intertile_tx(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, &dummy, sizeof(dummy));
#else
    int ret = 0;
    rtos_intertile_rx_len(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
    rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
#endif

    static device_control_servicer_t servicer_ctx;
    control_ret_t dc_ret;

	for (;;) {
        #if ON_TILE(0)
        {
            control_resid_t resources[] = {0x3};

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
            control_resid_t resources[] = {0x33};

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
      //usb_manager_start(appconfUSB_DEVICE_CTRL_TASK_PRIORITY);
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

#endif // #if RUN_EP0_VIA_PROXY

