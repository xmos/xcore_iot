// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            MIC_ARRAY_CONFIG_MCLK_FREQ
#define appconfPDM_CLOCK_FREQUENCY              MIC_ARRAY_CONFIG_PDM_FREQ
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       16000
#define appconfAUDIO_FRAME_LENGTH            	MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            (configMAX_PRIORITIES/2 + 5)
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    	(configMAX_PRIORITIES - 4)

#endif /* APP_CONF_H_ */
