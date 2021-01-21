// Copyright (c) 2020-2021, XMOS Ltd, All rights reserved

#include <string.h>

/* wfx headers */
#include "sl_wfx.h"
#include "FreeRTOS/sl_wfx_host.h"

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

#if (configTASK_NOTIFICATION_ARRAY_ENTRIES <= 1)
#error configTASK_NOTIFICATION_ARRAY_ENTRIES must be greater than 1 to use the sl_wfx_host
#endif

typedef enum sl_wfx_host_receive_task_notify_index_t {
    /* 0 is reserved for streambuffer/message buffer use */
    eNOTIFY_SL_WFX_HOST_BUS_IRQ_BM = 1,  ///<< Notification to receive frame
} sl_wfx_host_receive_task_notify_index_t;

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
    TickType_t ticks_to_wait = portMAX_DELAY;
    int error_count = 0;
    const int max_errors = 3;
    int reset = 0;

    for (;;) {
        /* Wait for an interrupt from WF200 */
        ret = xTaskNotifyWaitIndexed(eNOTIFY_SL_WFX_HOST_BUS_IRQ_BM,
                                     0x00000000,
                                     0xFFFFFFFF,
                                     &bits,
                                     ticks_to_wait);

        if (ret == pdFAIL && ticks_to_wait != portMAX_DELAY) {
            /*
             * The wait timed out. If a timeout value was set (not portMAX_DELAY)
             * then treat it as if an interrupt occurred.
             */
            bits = SL_WFX_HOST_BUS_IRQ_BM;
            ret = pdPASS;
            sl_wfx_host_log("Checking for frames again\n");
        }

        if (ret == pdPASS) {

            if (bits & SL_WFX_HOST_BUS_DEINIT_BM) {
                sl_wfx_host_log("WFX200 host task ending\n");
                vTaskDelete(NULL);
            }

            if (bits & SL_WFX_HOST_BUS_IRQ_BM) {
                sl_status_t result;

                /* Receive the frame(s) pending in WF200 */
                result = sl_wfx_host_receive_frames();
                switch (result) {
                case SL_STATUS_OK:
                    ticks_to_wait = portMAX_DELAY;
                    error_count = 0;
                    break;
                case SL_STATUS_WIFI_NO_PACKET_TO_RECEIVE:
                case SL_STATUS_TIMEOUT:
                    if (error_count++ == max_errors) {
                        reset = 1;
                        ticks_to_wait = portMAX_DELAY;
                    } else {
                        ticks_to_wait = pdMS_TO_TICKS(100);
                    }
                    break;
                default:
                    reset = 1;
                    ticks_to_wait = portMAX_DELAY;
                    break;
                }

                if (reset) {
                    reset = 0;
                    sl_wfx_host_log("Frame receive error %04x, will reset module\n", result);
                    sl_wfx_host_reset();
                }
            }
        }
    }
}

void sl_wfx_host_task_rx_notify(BaseType_t *xYieldRequired)
{
    int state = taskENTER_CRITICAL_FROM_ISR();
    if (receive_task_handle != NULL) {
        xEventGroupSetBitsFromISR(sl_wfx_event_group, SL_WFX_INTERRUPT, xYieldRequired);
        xTaskNotifyIndexedFromISR(receive_task_handle, eNOTIFY_SL_WFX_HOST_BUS_IRQ_BM, SL_WFX_HOST_BUS_IRQ_BM, eSetBits, xYieldRequired);
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
        xEventGroupClearBits(sl_wfx_event_group, SL_WFX_INITIALIZED);
        xTaskNotify(handle, SL_WFX_HOST_BUS_DEINIT_BM, eSetBits);
    }
}

void sl_wfx_host_task_start(void)
{
    /* create a mutex used for making driver accesses atomic */
    if (s_xDriverSemaphore == NULL) {
        s_xDriverSemaphore = xSemaphoreCreateRecursiveMutex ();
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
        xEventGroupClearBits(sl_wfx_event_group, 0x00FFFFFF);
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_INITIALIZED);

        xTaskCreate(sl_wfx_host_receive_task,
                    "sl_wfx_host_receive_task",
                    portTASK_STACK_DEPTH(sl_wfx_host_receive_task),
                    NULL,
					configMAX_PRIORITIES - 1,
                    &receive_task_handle);
        configASSERT(sl_wfx_host_receive_task != NULL);
    }
}
