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
SemaphoreHandle_t s_xDriverSemaphore = NULL;
EventGroupHandle_t sl_wfx_event_group = NULL;

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
    BaseType_t ret;

    for (;;) {
        /* Wait for an interrupt from WF200 */

        ret = xTaskNotifyWait(0x00000000,
                              0xFFFFFFFF,
                              &bits,
                              portMAX_DELAY);

        if (ret == pdPASS) {
            if (bits & SL_WFX_HOST_BUS_DEINIT_BM) {
                sl_wfx_host_log("WFX200 host task ending\n");
                vTaskDelete(NULL);
            }

            if (bits & SL_WFX_HOST_BUS_IRQ_BM) {
                /* Receive the frame(s) pending in WF200 */
                sl_wfx_host_receive_frames();
            }
        }
    }
}

void sl_wfx_host_task_rx_notify(BaseType_t *xYieldRequired)
{
    int state = taskENTER_CRITICAL_FROM_ISR();
    if (receive_task_handle != NULL) {
        xEventGroupSetBitsFromISR(sl_wfx_event_group, SL_WFX_INTERRUPT, xYieldRequired);
        xTaskNotifyFromISR(receive_task_handle, SL_WFX_HOST_BUS_IRQ_BM, eSetBits, xYieldRequired);
    }
    taskEXIT_CRITICAL_FROM_ISR(state);
}

void sl_wfx_host_task_stop(void)
{
    TaskHandle_t handle;

    taskENTER_CRITICAL();
    handle = receive_task_handle;
    receive_task_handle = NULL;
    taskEXIT_CRITICAL();

    if (handle != NULL) {
        xTaskNotify(handle, SL_WFX_HOST_BUS_DEINIT_BM, eSetBits);
    }
}

void sl_wfx_host_task_start(void)
{
    /* create a mutex used for making driver accesses atomic */
    if (s_xDriverSemaphore == NULL) {
        s_xDriverSemaphore = xSemaphoreCreateMutex();
        configASSERT(s_xDriverSemaphore != NULL);
    }

    /* create an event group to track Wi-Fi events */
    if (sl_wfx_event_group == NULL) {
        sl_wfx_event_group = xEventGroupCreate();
        configASSERT(sl_wfx_event_group != NULL);
    }

    if (receive_task_handle == NULL) {
        /* Ensure all event group bits are clear in case any are
        still set from a previous run. */
        xEventGroupClearBits( sl_wfx_event_group, 0x00FFFFFF );

        xTaskCreate(sl_wfx_host_receive_task,
                    "sl_wfx_host_receive_task",
                    portTASK_STACK_DEPTH(sl_wfx_host_receive_task),
                    NULL,
					configMAX_PRIORITIES - 1,
                    &receive_task_handle);
        configASSERT(sl_wfx_host_receive_task != NULL);
    }
}
