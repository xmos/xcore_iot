// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if appconfI2S_TDM_ENABLED && appconfI2S_AUDIO_SAMPLE_RATE != 3*appconfAUDIO_PIPELINE_SAMPLE_RATE
#error appconfI2S_AUDIO_SAMPLE_RATE must be 48000 to use I2S TDM
#endif

#endif /* APP_CONF_CHECK_H_ */
