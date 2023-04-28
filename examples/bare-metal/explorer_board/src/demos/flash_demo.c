// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* App headers */
#include "app_demos.h"

void flash_demo(void)
{
	uint32_t flash_size = fl_getFlashSize();
    debug_printf("Flash size: 0x%x\n", flash_size);
    while(1) {;}
}
