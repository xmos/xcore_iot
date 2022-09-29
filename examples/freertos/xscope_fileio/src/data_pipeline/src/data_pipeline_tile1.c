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

#if ON_TILE(1)

static void *data_pipeline_input_i(void *input_app_data)
{
    frame_data_t *frame_data;

    frame_data = pvPortMalloc(sizeof(frame_data_t));
    memset(frame_data, 0x00, sizeof(frame_data_t));

    data_pipeline_input(input_app_data,
                       (int8_t **)frame_data->data,
                       appconfDATA_FRAME_SIZE_BYTES);

    return frame_data;
}

static int data_pipeline_output_i(frame_data_t *frame_data,
                                   void *output_app_data)
{
    rtos_intertile_tx(intertile_ctx,
                      appconfEXAMPLE_DATA_PORT,
                      frame_data,
                      sizeof(frame_data_t));
    return DATA_PIPELINE_FREE_FRAME;
}

static void stage_preemption_disabled(frame_data_t *frame_data)
{
    uint32_t time_start, time_end;

    // Disable preemption around the performance critical code section that follows
    uint32_t mask = rtos_interrupt_mask_all();
    {
        time_start = get_reference_time();
        /* Apply a fixed gain to all samples */
        for (int i=0; i<appconfFRAME_ADVANCE; i++) {
            frame_data->data[i] *= 2;
        }
        time_end = get_reference_time();
    }
    rtos_interrupt_mask_set(mask); // Enable preemption
    
    rtos_printf("stage_preemption_disabled: %d (microseconds) \n", (time_end - time_start) / 100);
}

static void stage_preemption_enabled(frame_data_t *frame_data)
{
    uint32_t time_start, time_end;
    
    // Preemption is not disabled around the code section that follows
    //   Instead, the code periodically yields to the RTOS kernel to 
    //   emulate a task context switch.

    time_start = get_reference_time();
    /* Apply a fixed gain to all samples */
    for (int i=0; i<appconfFRAME_ADVANCE; i++) {
        frame_data->data[i] *= 2;
        if (i % 100 == 0) {
            // Yield to the RTOS kernel here
            taskYIELD();
        }
    }
    time_end = get_reference_time();

    rtos_printf("stage_preemption_enabled: %d (microseconds) \n", (time_end - time_start) / 100);
}

void data_pipeline_init(
    void *input_app_data,
    void *output_app_data)
{
    const int stage_count = 2;

    const pipeline_stage_t stages[] = {
        (pipeline_stage_t)stage_preemption_disabled,
        (pipeline_stage_t)stage_preemption_enabled,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_preemption_disabled) + RTOS_THREAD_STACK_SIZE(data_pipeline_input_i),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage_preemption_enabled) + RTOS_THREAD_STACK_SIZE(data_pipeline_output_i),
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

#endif /* ON_TILE(1)*/
