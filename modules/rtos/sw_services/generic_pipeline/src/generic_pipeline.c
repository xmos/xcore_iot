// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#define DEBUG_UNIT SW_SERVICE_GENERIC_PIPELINE

#include <xcore/assert.h>
#include "rtos_osal.h"
#include "generic_pipeline.h"

typedef struct {
	pipeline_input_t input;
	pipeline_output_t output;
	void *input_data;
	void *output_data;
	rtos_osal_queue_t *queues;
	int stage_count;
} pipeline_ctx_t;

typedef struct {
	pipeline_ctx_t *pipeline_ctx;
	pipeline_stage_t stage_function;
	int stage;
} pipeline_stage_ctx_t;

static void generic_pipeline_stage(pipeline_stage_ctx_t *ctx)
{
	void *generic_frame_buffer;
	pipeline_ctx_t *pipeline_ctx = ctx->pipeline_ctx;
	rtos_osal_queue_t *input_queue;
	rtos_osal_queue_t *output_queue;

	if (ctx->stage > 0) {
		input_queue = &pipeline_ctx->queues[ctx->stage - 1];
	} else {
		input_queue = NULL;
	}

	if (ctx->stage < pipeline_ctx->stage_count - 1) {
		output_queue = &pipeline_ctx->queues[ctx->stage];
	} else {
		output_queue = NULL;
	}

	for (;;) {

		if (input_queue != NULL) {
           (void) rtos_osal_queue_receive(input_queue, &generic_frame_buffer, RTOS_OSAL_WAIT_FOREVER);
		} else {
			generic_frame_buffer = pipeline_ctx->input(pipeline_ctx->input_data);
			if (generic_frame_buffer == NULL) {
				continue;
			}
		}

		ctx->stage_function(generic_frame_buffer);

		if (output_queue != NULL) {
            (void) rtos_osal_queue_send(output_queue, &generic_frame_buffer, RTOS_OSAL_WAIT_FOREVER);
		} else {
			if (pipeline_ctx->output(generic_frame_buffer, pipeline_ctx->output_data) != 0) {
				rtos_osal_free(generic_frame_buffer);
			}
		}

		(void) rtos_osal_task_yield();
	}
}

void generic_pipeline_init(
		const pipeline_input_t input,
		const pipeline_output_t output,
		void * const input_data,
		void * const output_data,
		const pipeline_stage_t * const stage_functions,
		const size_t * const stage_stack_sizes,
		const int pipeline_priority,
		const int stage_count)
{
	pipeline_ctx_t *pipeline_ctx;
	pipeline_stage_ctx_t *stage_ctx;
	int i;
    char stage_name[7] = "stage0\0";

	pipeline_ctx = rtos_osal_malloc(sizeof(pipeline_ctx_t));
	pipeline_ctx->input = input;
	pipeline_ctx->input_data = input_data;
	pipeline_ctx->output_data = output_data;
	pipeline_ctx->output = output;
	pipeline_ctx->stage_count = stage_count;
    if (stage_count > 1) {
    	pipeline_ctx->queues = rtos_osal_malloc((stage_count - 1) * sizeof(rtos_osal_queue_t));

    	for (i = 0; i < stage_count - 1; i++) {
            (void) rtos_osal_queue_create(&pipeline_ctx->queues[i], NULL, 2, sizeof(void *));
    	}
    } else {
        pipeline_ctx->queues = NULL;
    }

	stage_ctx = rtos_osal_malloc(stage_count * sizeof(pipeline_stage_ctx_t));

	for (i = 0; i < stage_count; i++) {
        stage_ctx[i].pipeline_ctx = pipeline_ctx;
        stage_ctx[i].stage = i;
        stage_ctx[i].stage_function = stage_functions[i];

        xassert(stage_count < 10); /* Name will still be unique but limit to 0-9 ASCII */
        stage_name[5] = i + '0';

        (void) rtos_osal_thread_create(
                (rtos_osal_thread_t *) NULL,
                (char *) stage_name,
                (rtos_osal_entry_function_t) generic_pipeline_stage,
                (void *) &stage_ctx[i],
                (size_t) stage_stack_sizes[i],
                (unsigned int) pipeline_priority);
	}
}
