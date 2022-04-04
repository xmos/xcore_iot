// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_PLL_CTRL_H_
#define APP_PLL_CTRL_H_

#include "platform_conf.h"

#if (MIC_ARRAY_CONFIG_MCLK_FREQ != 24576000)
#error PLL values only valid if MIC_ARRAY_CONFIG_MCLK_FREQ == 24576000
#endif

#define APP_PLL_CTL_VAL   0x0A019803 // Valid for all fractional values
#define APP_PLL_FRAC_NOM  0x800095F9 // 24.576000 MHz

void app_pll_set_numerator(int numerator);
void app_pll_init(void);

#endif /* APP_PLL_CTRL_H_ */
