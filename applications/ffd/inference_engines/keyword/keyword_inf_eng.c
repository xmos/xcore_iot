// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "event_groups.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "keyword_inf_eng.h"

void keyword_engine_task(keyword_engine_args_t *args)
{
    StreamBufferHandle_t input_buf = args->samples_to_engine_stream_buf;
    EventGroupHandle_t output_egrp = args->egrp_inference;

    int32_t buf[appconfINFERENCE_FRAMES_PER_INFERENCE];

    /* Perform any initialization here */
    int dummy_output_test = 0;

    while (1)
    {
        /* Receive audio frames */
        uint8_t *buf_ptr = (uint8_t*)buf;
        size_t buf_len = appconfINFERENCE_FRAMES_PER_INFERENCE * sizeof(int32_t);
        do {
            size_t bytes_rxed = xStreamBufferReceive(input_buf,
                                                     buf_ptr,
                                                     buf_len,
                                                     portMAX_DELAY);
            buf_len -= bytes_rxed;
            buf_ptr += bytes_rxed;
        } while(buf_len > 0);

        /* Perform inference here */
        rtos_printf("inference\n");

        /* Set dummy output event bits */
        switch(dummy_output_test)
        {
            default:
            case 0:
                xEventGroupSetBits(output_egrp, INFERENCE_BIT_A | INFERENCE_BIT_B);
                break;
            case 1:
                xEventGroupSetBits(output_egrp, INFERENCE_BIT_A);
                break;
            case 2:
                xEventGroupSetBits(output_egrp, INFERENCE_BIT_B);
                break;
        }
        dummy_output_test = (dummy_output_test >= 2) ? 0 : dummy_output_test + 1;
    }
}
