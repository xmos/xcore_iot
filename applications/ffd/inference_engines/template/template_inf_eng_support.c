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

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "template_inf_eng.h"

static StreamBufferHandle_t samples_to_engine_stream_buf = 0;

void template_engine_samples_send_remote(
        rtos_intertile_t *intertile_ctx,
        size_t frame_count,
        int32_t *processed_audio_frame)
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    rtos_intertile_tx(intertile_ctx,
                      appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                      processed_audio_frame,
                      sizeof(int32_t) * frame_count);
}

static void template_engine_intertile_samples_in_task(void *arg)
{
    (void) arg;

    for (;;) {
        int32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
        size_t bytes_received;

        bytes_received = rtos_intertile_rx_len(
                intertile_ctx,
                appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                portMAX_DELAY);

        xassert(bytes_received == sizeof(samples));

        rtos_intertile_rx_data(
                intertile_ctx,
                samples,
                bytes_received);

        if (xStreamBufferSend(samples_to_engine_stream_buf, samples, sizeof(samples), 0) != sizeof(samples)) {
            rtos_printf("lost output samples for inference\n");
        }
    }
}

void template_engine_samples_send_local(
        size_t frame_count,
        int32_t *processed_audio_frame)
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    if(samples_to_engine_stream_buf != NULL) {
        size_t bytes_to_send = sizeof(int32_t) * frame_count;
        if (xStreamBufferSend(samples_to_engine_stream_buf, processed_audio_frame, bytes_to_send, 0) != bytes_to_send) {
            rtos_printf("lost local output samples for inference\n");
        }
    } else {
        rtos_printf("inference engine streambuffer not ready\n");
    }
}

void template_engine_task_create(unsigned priority)
{
    samples_to_engine_stream_buf = xStreamBufferCreate(
                                           2 * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINFERENCE_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)template_engine_task,
                "template_eng",
                RTOS_THREAD_STACK_SIZE(template_engine_task),
                samples_to_engine_stream_buf,
                uxTaskPriorityGet(NULL),
                NULL);
}

void template_engine_intertile_task_create(uint32_t priority)
{
    samples_to_engine_stream_buf = xStreamBufferCreate(
                                           appconfINFERENCE_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINFERENCE_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)template_engine_intertile_samples_in_task,
                "inf_intertile_rx",
                RTOS_THREAD_STACK_SIZE(template_engine_intertile_samples_in_task),
                NULL,
                priority-1,
                NULL);

    xTaskCreate((TaskFunction_t)template_engine_task,
                "template_eng",
                RTOS_THREAD_STACK_SIZE(template_engine_task),
                samples_to_engine_stream_buf,
                uxTaskPriorityGet(NULL),
                NULL);
}
