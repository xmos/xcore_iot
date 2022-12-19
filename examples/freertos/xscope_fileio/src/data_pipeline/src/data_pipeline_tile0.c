// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "generic_pipeline.h"

/* App headers */
#include "app_conf.h"
#include "data_pipeline.h"

#if ON_TILE(0)

static void *data_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));
    memset(frame_data, 0x00, sizeof(frame_data_t));

    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfEXAMPLE_DATA_PORT,
            portMAX_DELAY);

    xassert(bytes_received == sizeof(frame_data_t));

    rtos_intertile_rx_data(
            intertile_ctx,
            frame_data,
            bytes_received);

    return frame_data;
}

static int data_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    return data_pipeline_output(output_app_data,
                               (int8_t **)frame_data->data,
                               appconfDATA_FRAME_SIZE_BYTES);
}

static void stage_3(frame_data_t *frame_data)
{
    /* Do nothing */
}

void data_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
    const int stage_count = 1;

    const pipeline_stage_t stages[] = {
        (pipeline_stage_t) stage_3,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_3) + RTOS_THREAD_STACK_SIZE(data_pipeline_input_i) + RTOS_THREAD_STACK_SIZE(data_pipeline_output_i),
    };

    generic_pipeline_init((pipeline_input_t)data_pipeline_input_i,
                        (pipeline_output_t)data_pipeline_output_i,
                        input_app_data,
                        output_app_data,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        appconfDATA_PIPELINE_TASK_PRIORITY,
                        stage_count);
}

#endif /* ON_TILE(0)*/
