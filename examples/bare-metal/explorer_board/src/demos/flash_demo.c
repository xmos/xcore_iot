// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"
#include "qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"

void flash_demo(qspi_flash_ctx_t *qspi_flash_ctx)
{
	uint32_t id_reg;
	qspi_flash_read_id(qspi_flash_ctx, (uint8_t*)&id_reg, 4);
    debug_printf("flash id: 0x%x\n", id_reg);
    while(1) {;}
}
