// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if !XCOREAI_EXPLORER
#error Only the XCORE-AI-EXPLORER Board is supported
#endif

#if appconfI2S_MODE != appconfI2S_MODE_MASTER
#error I2S mode other than master is not currently supported
#endif

#if (appconfUSB_INPUT + appconfMIC_INPUT + appconfI2S_INPUT) > 1
#error Only 1 audio input mode can be selected
#endif

#if (appconfUSB_INPUT + appconfMIC_INPUT + appconfI2S_INPUT) == 0
#error At least 1 audio input mode must be selected
#endif

#endif /* APP_CONF_CHECK_H_ */
