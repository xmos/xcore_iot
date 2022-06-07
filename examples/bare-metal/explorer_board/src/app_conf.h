// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Audio Pipeline Configuration */
#define appconfAUDIO_FRAME_LENGTH            	MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME
#define appconfMIC_COUNT                        MIC_ARRAY_CONFIG_MIC_COUNT
#define appconfFRAMES_IN_ALL_CHANS              (appconfAUDIO_FRAME_LENGTH * appconfMIC_COUNT)
#define appconfEXP                              -31
#define appconfINITIAL_GAIN                     20
#define appconfAUDIO_PIPELINE_MAX_GAIN          60
#define appconfAUDIO_PIPELINE_MIN_GAIN          0
#define appconfAUDIO_PIPELINE_GAIN_STEP         4
#define appconfPOWER_THRESHOLD                  (float)0.00001
#define appconfAUDIO_CLOCK_FREQUENCY            24576000
#define appconfPDM_CLOCK_FREQUENCY              3072000
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       16000

#endif /* APP_CONF_H_ */
