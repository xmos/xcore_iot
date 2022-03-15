// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "ww_model_runner/ww_model_runner.h"

#define ASR_CHANNEL             (0)
#define COMMS_CHANNEL           (1)

#if appconfWW_ENABLED
extern configSTACK_DEPTH_TYPE model_runner_manager_stack_size;

void ww_audio_send(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t (*processed_audio_frame)[2])
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    uint16_t ww_samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        ww_samples[i] = (uint16_t)(processed_audio_frame[i][ASR_CHANNEL] >> 16);
    }

    rtos_intertile_tx(intertile_ctx,
                      appconfWW_SAMPLES_PORT,
                      ww_samples,
                      sizeof(ww_samples));
}

void ww_samples_in_task(void *arg)
{
    StreamBufferHandle_t samples_to_ww_stream_buf = (StreamBufferHandle_t) arg;

    for (;;) {
        uint16_t ww_samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
        size_t bytes_received;

        bytes_received = rtos_intertile_rx_len(
                intertile_ctx,
                appconfWW_SAMPLES_PORT,
                portMAX_DELAY);

        xassert(bytes_received == sizeof(ww_samples));

        rtos_intertile_rx_data(
                intertile_ctx,
                ww_samples,
                bytes_received);

        if (xStreamBufferSend(samples_to_ww_stream_buf, ww_samples, sizeof(ww_samples), 0) != sizeof(ww_samples)) {
            rtos_printf("lost output samples for ww\n");
        }
    }
}

void ww_task_create(unsigned priority)
{
    StreamBufferHandle_t audio_stream = xStreamBufferCreate(
                                               2 * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                               appconfWW_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)ww_samples_in_task,
                "ww_audio_rx",
                RTOS_THREAD_STACK_SIZE(ww_samples_in_task),
                audio_stream,
                priority-1,
                NULL);

    xTaskCreate((TaskFunction_t)model_runner_manager,
                "model_manager",
                model_runner_manager_stack_size,
                audio_stream,
                uxTaskPriorityGet(NULL),
                NULL);
}

#endif
