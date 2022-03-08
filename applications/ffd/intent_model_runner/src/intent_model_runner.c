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
#include "intent_model_runner.h"

#define ASR_CHANNEL             (0)
#define COMMS_CHANNEL           (1)

static StreamBufferHandle_t samples_to_intent_stream_buf = 0;

extern configSTACK_DEPTH_TYPE model_runner_manager_stack_size;

void intent_intertile_samples_send(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t (*processed_audio_frame)[2])
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        samples[i] = (uint32_t)processed_audio_frame[i][ASR_CHANNEL];
    }

    rtos_intertile_tx(intertile_ctx,
                      appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                      samples,
                      sizeof(samples));
}

void intent_intertile_samples_in_task(void *arg)
{
    (void) arg;

    for (;;) {
        uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
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

        if (xStreamBufferSend(samples_to_intent_stream_buf, samples, sizeof(samples), 0) != sizeof(samples)) {
            rtos_printf("lost output samples for intent\n");
        }
    }
}

void intent_samples_send(size_t frame_count,
                         int32_t (*processed_audio_frame)[2])
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        samples[i] = (uint32_t)(processed_audio_frame[i][ASR_CHANNEL]);
    }

    if (xStreamBufferSend(samples_to_intent_stream_buf, processed_audio_frame, sizeof(samples), 0) != sizeof(samples)) {
        rtos_printf("lost local output samples for intent\n");
    }
}

void intent_task_create(unsigned priority)
{
    samples_to_intent_stream_buf = xStreamBufferCreate(
                                           2 * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINTENT_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)model_runner_manager,
                "model_manager",
                model_runner_manager_stack_size,
                samples_to_intent_stream_buf,
                uxTaskPriorityGet(NULL),
                NULL);
}

void intertile_intent_task_create(unsigned priority)
{
    samples_to_intent_stream_buf = xStreamBufferCreate(
                                           appconfINTENT_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINTENT_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)intent_intertile_samples_in_task,
                "intent_audio_rx",
                RTOS_THREAD_STACK_SIZE(intent_intertile_samples_in_task),
                NULL,
                priority-1,
                NULL);

    xTaskCreate((TaskFunction_t)model_runner_manager,
                "model_manager",
                model_runner_manager_stack_size,
                samples_to_intent_stream_buf,
                uxTaskPriorityGet(NULL),
                NULL);
}
