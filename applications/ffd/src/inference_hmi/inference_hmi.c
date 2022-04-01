// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "keyword_inf_eng.h"
#include "inference_hmi/inference_hmi.h"

void inference_hmi_task(void *args)
{
    EventGroupHandle_t egrp_intent = (EventGroupHandle_t) args;
    EventBits_t rx_bits = 0;

    while(1)
    {
        /* Wait forever for a bit change.  Clear changed bit on exit */
        rx_bits = xEventGroupWaitBits(
            egrp_intent,
            INFERENCE_BIT_A | INFERENCE_BIT_B,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

        if((rx_bits & (INFERENCE_BIT_A | INFERENCE_BIT_B)) == (INFERENCE_BIT_A | INFERENCE_BIT_B))
        {
            rtos_printf("intent_hmi: both bits set\n");
        } else if ((rx_bits & INFERENCE_BIT_A) != 0) {
            rtos_printf("intent_hmi: bit A set\n");
        } else if ((rx_bits & INFERENCE_BIT_B) != 0) {
            rtos_printf("intent_hmi: bit B set\n");
        } else {
            configASSERT(0); /* Should never get here */
        }
    }
}


void inference_hmi_create(unsigned priority, void *args)
{
    xTaskCreate((TaskFunction_t) inference_hmi_task,
                "inf_hmi",
                RTOS_THREAD_STACK_SIZE(inference_hmi_task),
                args,
                priority,
                NULL);
}
