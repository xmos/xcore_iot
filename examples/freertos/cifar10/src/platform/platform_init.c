// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
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

    qspi_flash_ctx->ctx.sfdp_skip = true;
    qspi_flash_ctx->ctx.sfdp_supported = false;
    qspi_flash_ctx->ctx.page_size_bytes = 256;
    qspi_flash_ctx->ctx.page_count = 16384;
    qspi_flash_ctx->ctx.flash_size_kbytes = 4096;
    qspi_flash_ctx->ctx.address_bytes = 3;
    qspi_flash_ctx->ctx.erase_info[0].size_log2 = 12;
    qspi_flash_ctx->ctx.erase_info[0].cmd = 0xEEFEEEEE;
    qspi_flash_ctx->ctx.erase_info[1].size_log2 = 15;
    qspi_flash_ctx->ctx.erase_info[1].cmd = 0xEFEFEEFE;
    qspi_flash_ctx->ctx.erase_info[2].size_log2 = 16;
    qspi_flash_ctx->ctx.erase_info[2].cmd = 0xFFEFFEEE;
    qspi_flash_ctx->ctx.erase_info[3].size_log2 = 0;
    qspi_flash_ctx->ctx.erase_info[3].cmd = 0;
    qspi_flash_ctx->ctx.busy_poll_cmd = 0xEEEEEFEF;
    qspi_flash_ctx->ctx.busy_poll_bit = 0;
    qspi_flash_ctx->ctx.busy_poll_ready_value = 0;
    qspi_flash_ctx->ctx.qe_reg = 2;
    qspi_flash_ctx->ctx.qe_bit = 1;
    qspi_flash_ctx->ctx.sr2_read_cmd = 0xEEFFEFEF;
    qspi_flash_ctx->ctx.sr2_write_cmd = 0xEEEEEEEE;

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
