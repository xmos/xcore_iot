/*
 * audio_pipeline.h
 *
 *  Created on: Oct 23, 2019
 *      Author: jmccarthy
 */


#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

void audio_pipeline_create(QueueHandle_t output, UBaseType_t priority);

BaseType_t audiopipeline_get_stage1_gain( void );
BaseType_t audiopipeline_set_stage1_gain( BaseType_t xnewgain );

#endif /* AUDIO_PIPELINE_H_ */
