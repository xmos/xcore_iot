// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "fs_support.h"

static void flash_start(void)
{
    rtos_qspi_flash_rpc_config(qspi_flash_ctx, QSPI_RPC_PORT, QSPI_RPC_HOST_TASK_PRIORITY);
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
    rtos_fatfs_init(qspi_flash_ctx);
#endif

#if ON_TILE(1)
    #if USE_SWMEM
        swmem_setup(qspi_flash_ctx, appconfSWMEM_TASK_PRIORITY);
    #endif
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);

    flash_start();
}
