// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

typedef struct ap_stage
{
	QueueHandle_t input;
	QueueHandle_t output;
	void * args;
} ap_stage_t;

typedef struct ap_stage * ap_stage_handle_t;

void audio_pipeline_create( UBaseType_t priority);

BaseType_t audiopipeline_get_stage1_gain( void );
BaseType_t audiopipeline_set_stage1_gain( BaseType_t xnewgain );

#endif /* AUDIO_PIPELINE_H_ */
