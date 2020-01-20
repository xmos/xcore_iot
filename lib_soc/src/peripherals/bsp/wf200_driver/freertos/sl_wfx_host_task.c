// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#define SL_WFX_HOST_BUS_IRQ_BM    0x00000001
#define SL_WFX_HOST_BUS_DEINIT_BM 0x00000002

static TaskHandle_t receive_task_handle;
SemaphoreHandle_t s_xDriverSemaphore;
EventGroupHandle_t sl_wfx_event_group;

static sl_status_t sl_wfx_host_receive_frames(void)
{
    sl_status_t result;
    uint16_t control_register = 0;

    do {
        result = sl_wfx_receive_frame(&control_register);
        SL_WFX_ERROR_CHECK(result);
    } while ((control_register & SL_WFX_CONT_NEXT_LEN_MASK) != 0);

    error_handler:
    return result;
}

static void sl_wfx_host_receive_task(void *arg)
{
    uint32_t bits;

    /* create a mutex used for making driver accesses atomic */
    //s_xDriverSemaphore = xSemaphoreCreateMutex();

    /* create an event group to track Wi-Fi events */
    //sl_wfx_event_group = xEventGroupCreate();

    for (;;) {
        /* Wait for an interrupt from WF200 */
        /* TODO: check return value */
        xTaskNotifyWait(0x00000000,
                        0xFFFFFFFF,
                        &bits,
                        portMAX_DELAY);

        if (bits & SL_WFX_HOST_BUS_DEINIT_BM) {
            sl_wfx_host_log("WFX200 host task ending\n");
            vTaskDelete(receive_task_handle);
        }

        if (bits & SL_WFX_HOST_BUS_IRQ_BM) {
            /* Receive the frame(s) pending in WF200 */
            sl_wfx_host_receive_frames();
        }
    }
}

void sl_wfx_host_task_rx_notify(BaseType_t *xYieldRequired)
{
    xTaskNotifyFromISR(receive_task_handle, SL_WFX_HOST_BUS_IRQ_BM, eSetBits, xYieldRequired);
}

void sl_wfx_host_task_stop(void)
{
    xTaskNotify(receive_task_handle, SL_WFX_HOST_BUS_DEINIT_BM, eSetBits);
}

void sl_wfx_host_task_start(void)
{
    /* create a mutex used for making driver accesses atomic */
    s_xDriverSemaphore = xSemaphoreCreateMutex();

    /* create an event group to track Wi-Fi events */
    sl_wfx_event_group = xEventGroupCreate();

    xTaskCreate(sl_wfx_host_receive_task,
                "sl_wfx_host_receive_task",
                portTASK_STACK_DEPTH(sl_wfx_host_receive_task),
                NULL,
                15,
                &receive_task_handle);
}
