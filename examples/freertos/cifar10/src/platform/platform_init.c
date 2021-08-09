// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>

#include "app_conf.h"
#include "platform/driver_instances.h"

/** TILE 0 Clock Blocks */
#define FLASH_CLKBLK  XS1_CLKBLK_1

static void flash_init(void)
{
    static rtos_driver_rpc_t qspi_flash_rpc_config;

#if ON_TILE(FLASH_TILE_NO)
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,

            /** Derive QSPI clock from the 600 MHz xcore clock **/
            qspi_io_source_clock_xcore,

            /** Full speed clock configuration **/
            5, // 600 MHz / (2*5) -> 60 MHz,
            1,
            qspi_io_sample_edge_rising,
            0,

            /** SPI read clock configuration **/
            12, // 600 MHz / (2*12) -> 25 MHz
            0,
            qspi_io_sample_edge_falling,
            0,

            qspi_flash_page_program_1_4_4);

    rtos_qspi_flash_rpc_host_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            client_intertile_ctx,
            1);
#else
    rtos_qspi_flash_rpc_client_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            intertile_ctx);
#endif
}

void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);

    flash_init();
}
