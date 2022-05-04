//// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _MDOB128032GV_H_
#define _MDOB128032GV_H_

#include "ssd1306.h"

extern ssd1306_display MDOB128032GV;

#define MDOB128032GV_COLS (128)
#define MDOB128032GV_ROWS (32)

// The display is offset by a few columns
#define MDOB128032GV_OFFSET (96)

#endif // _MDOB128032GV_H_
