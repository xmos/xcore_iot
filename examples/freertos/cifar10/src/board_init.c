// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "board_init.h"
#include "app_conf.h"

static rtos_driver_rpc_t qspi_flash_rpc_config;

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    rtos_intertile_init(intertile_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            XS1_CLKBLK_2,
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

            qspi_flash_page_program_1_1_1);

    rtos_qspi_flash_rpc_host_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            client_intertile_ctx,
            1);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx)
{
    rtos_intertile_init(intertile_ctx, tile0);

    rtos_qspi_flash_rpc_client_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            intertile_ctx);
}
