// Copyright (c) 2020 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef SRC_AUDIO_PIPELINE_AUDIO_PIPELINE_H_
#define SRC_AUDIO_PIPELINE_AUDIO_PIPELINE_H_


typedef void * (*audio_pipeline_input_t)(void *data);
typedef int (*audio_pipeline_output_t)(void *audio_frame_buffer, void *data);

typedef void (*audio_pipeline_stage_t)(void *audio_frame_buffer);

void audio_pipeline_init(
		const audio_pipeline_input_t input,
		const audio_pipeline_output_t output,
		void * const input_data,
		void * const output_data,
		const audio_pipeline_stage_t * const stage_functions,
		const configSTACK_DEPTH_TYPE * const stage_stack_sizes,
		const int pipeline_priority,
		const int stage_count);

#endif /* SRC_AUDIO_PIPELINE_AUDIO_PIPELINE_H_ */
