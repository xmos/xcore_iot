// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

void audio_pipeline_create(QueueHandle_t output0, QueueHandle_t output1, UBaseType_t priority);

BaseType_t audiopipeline_get_stage1_gain( void );
BaseType_t audiopipeline_set_stage1_gain( BaseType_t xnewgain );

#endif /* AUDIO_PIPELINE_H_ */
