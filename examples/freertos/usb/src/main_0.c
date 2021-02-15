// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <stdbool.h>
#include <platform.h>
#include <xs1.h>
#include <xcore/triggerable.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "demo_main.h"
#include "rtos_usb.h"
#include "rtos_interrupt.h"
#include "usb_support.h"

#if ON_TILE(0)
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;

static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

// temporary
void dcd_xcore_int_handler(rtos_usb_t *ctx,
                           void *app_data,
                           uint32_t ep_address,
                           size_t xfer_len,
                           int is_setup,
                           XUD_Result_t res);

void endpoint_0(rtos_usb_t *ctx);

uint8_t g_reportBuffer[4] = {0, 0, 0, 0};
void endpoint_1(rtos_usb_t *ctx)
{
    XUD_Result_t res;
    int state = 0;

    while (rtos_usb_endpoint_ready(ctx, 0x81, RTOS_OSAL_WAIT_FOREVER) != XUD_RES_OKAY);

    while (1) {
        g_reportBuffer[1] = 0;
        g_reportBuffer[2] = 0;

        /* Move the pointer around in a square (relative) */

        if (state == 0) {
            g_reportBuffer[1] = 40;
            g_reportBuffer[2] = 0;
            state += 1;
        } else if (state == 1) {
            g_reportBuffer[1] = 0;
            g_reportBuffer[2] = 40;
            state += 1;
        } else if (state == 2) {
            g_reportBuffer[1] = -40;
            g_reportBuffer[2] = 0;
            state += 1;
        } else if (state == 3) {
            g_reportBuffer[1] = 0;
            g_reportBuffer[2] = -40;
            state = 0;
        }

        /* Send the buffer off to the host.  Note this will return when complete */
        rtos_printf("Transfer on ep1...\n");
        res = rtos_usb_endpoint_transfer_start(ctx, 0x81, g_reportBuffer, 4);
        if (res == XUD_RES_OKAY) {
            res = rtos_usb_endpoint_transfer_complete(ctx, 0x81, NULL, RTOS_OSAL_WAIT_FOREVER);
        }
        if (res == XUD_RES_OKAY) {
            rtos_printf("done on ep1\n");
        } else if (res == XUD_RES_RST) {
            rtos_printf("Reset on EP1\n");
        }

        vTaskDelay(10);
    }
}

void vApplicationDaemonTaskStartup(void *arg)
{
    /* Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
     * if the endpoint wishes to be informed of USB bus resets
     */
//    XUD_EpType epTypeTableOut[2] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_DIS};
//    XUD_EpType epTypeTableIn[2] =   {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_INT};
//
//    rtos_usb_start(&usb_ctx,
//                   dcd_xcore_int_handler, NULL,
//                   2,
//                   epTypeTableOut,
//                   epTypeTableIn,
//                   XUD_SPEED_HS,
//                   XUD_PWR_BUS,
//                   configMAX_PRIORITIES - 1);

//    rtos_osal_thread_create(
//            NULL,
//            "endpoint_0",
//            (rtos_osal_entry_function_t) endpoint_0,
//            &usb_ctx,
//            RTOS_THREAD_STACK_SIZE(endpoint_0),
//            configMAX_PRIORITIES / 2);
//
//    rtos_osal_thread_create(
//            NULL,
//            "endpoint_1",
//            (rtos_osal_entry_function_t) endpoint_1,
//            &usb_ctx,
//            RTOS_THREAD_STACK_SIZE(endpoint_1),
//            configMAX_PRIORITIES / 2);

    rtos_printf("Starting intertile driver\n");
    rtos_intertile_start(intertile_ctx);

    /* Initialize drivers  */
    rtos_printf("Starting GPIO driver\n");
    rtos_gpio_start(gpio_ctx);

    rtos_printf("Starting QSPI flash driver\n");
    rtos_qspi_flash_start(qspi_flash_ctx, appconfRTOS_QSPI_FLASH_TASK_PRIORITY);

    usb_manager_start(appconfUSB_MANAGER_TASK_PRIORITY);
    create_tinyusb_demo(gpio_ctx, appconfTINYUSB_DEMO_TASK_PRIORITY);

    vTaskDelete(NULL);
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1, intertile_ctx, qspi_flash_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    rtos_printf("Starting startup task on tile 0 with %d stack\n", RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup) * 4);
    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
				RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
				appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("Start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

#endif /* ON_TILE(0) */
