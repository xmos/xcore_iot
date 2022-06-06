// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            24576000
#define appconfPDM_CLOCK_FREQUENCY              3072000
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       16000
#define appconfINITIAL_GAIN                     (float)20.0
#define appconfPOWER_THRESHOLD                  (float)0.00001
#define appconfAUDIO_FRAME_LENGTH            	256
#define appconfMIC_COUNT                        2
#define appconfFRAMES_IN_ALL_CHANS              (appconfAUDIO_FRAME_LENGTH * appconfMIC_COUNT)
#define appconfEXP                              -31

#endif /* APP_CONF_H_ */
