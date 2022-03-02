// Copyright (c) 2020 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "audio_pipeline.h"

typedef struct {
	audio_pipeline_input_t input;
	audio_pipeline_output_t output;
	void *input_data;
	void *output_data;
	QueueHandle_t *queues;
	int stage_count;
} audio_pipeline_ctx_t;

typedef struct {
	audio_pipeline_ctx_t *pipeline_ctx;
	audio_pipeline_stage_t stage_function;
	int stage;
} audio_pipeline_stage_ctx_t;

static void audio_pipeline_stage(audio_pipeline_stage_ctx_t *ctx)
{
	void *audio_frame_buffer;
	audio_pipeline_ctx_t *pipeline_ctx = ctx->pipeline_ctx;
	QueueHandle_t input_queue;
	QueueHandle_t output_queue;

	if (ctx->stage > 0) {
		input_queue = pipeline_ctx->queues[ctx->stage - 1];
	} else {
		input_queue = NULL;
	}

	if (ctx->stage < pipeline_ctx->stage_count - 1) {
		output_queue = pipeline_ctx->queues[ctx->stage];
	} else {
		output_queue = NULL;
	}

	for (;;) {

		if (input_queue != NULL) {
			xQueueReceive(input_queue, &audio_frame_buffer, portMAX_DELAY);
		} else {
			audio_frame_buffer = pipeline_ctx->input(pipeline_ctx->input_data);
			if (audio_frame_buffer == NULL) {
				continue;
			}
		}

		ctx->stage_function(audio_frame_buffer);

		if (output_queue != NULL) {
			xQueueSend(output_queue, &audio_frame_buffer, portMAX_DELAY);
		} else {
			if (pipeline_ctx->output(audio_frame_buffer, pipeline_ctx->output_data) != 0) {
				vPortFree(audio_frame_buffer);
			}
		}

		taskYIELD();
	}
}

void audio_pipeline_init(
		const audio_pipeline_input_t input,
		const audio_pipeline_output_t output,
		void * const input_data,
		void * const output_data,
		const audio_pipeline_stage_t * const stage_functions,
		const configSTACK_DEPTH_TYPE * const stage_stack_sizes,
		const int pipeline_priority,
		const int stage_count)
{
	audio_pipeline_ctx_t *pipeline_ctx;
	audio_pipeline_stage_ctx_t *stage_ctx;
	int i;
    char apstage_name[configMAX_TASK_NAME_LEN] = "apstage0";

	pipeline_ctx = pvPortMalloc(sizeof(audio_pipeline_ctx_t));
	pipeline_ctx->input = input;
	pipeline_ctx->input_data = input_data;
	pipeline_ctx->output_data = output_data;
	pipeline_ctx->output = output;
	pipeline_ctx->stage_count = stage_count;
	pipeline_ctx->queues = pvPortMalloc((stage_count - 1) * sizeof(QueueHandle_t));

	for (i = 0; i < stage_count - 1; i++) {
		pipeline_ctx->queues[i] = xQueueCreate(2, sizeof(void *));
	}

	stage_ctx = pvPortMalloc(stage_count * sizeof(audio_pipeline_stage_ctx_t));

	for (i = 0; i < stage_count; i++) {
        stage_ctx[i].pipeline_ctx = pipeline_ctx;
        stage_ctx[i].stage = i;
        stage_ctx[i].stage_function = stage_functions[i];

        configASSERT(stage_count < 10); /* Name will still be unique but limit to 0-9 ASCII */
        apstage_name[7] = i + '0';

		xTaskCreate(
                (TaskFunction_t) audio_pipeline_stage, /* Function that implements the task. */
				apstage_name,                          /* Text name for the task. */
				stage_stack_sizes[i],                  /* Stack size in words, not bytes. */
				&stage_ctx[i],                         /* Parameter passed into the task. */
				pipeline_priority,                     /* Priority at which the task is created. */
				NULL);                                 /* Don't need the created task's handle. */
	}
}
