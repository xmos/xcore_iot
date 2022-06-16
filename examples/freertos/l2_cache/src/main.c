// Copyright 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"

#include "rtos_qspi_flash.h"
#include "rtos_l2_cache.h"

#include "l2_cache.h"

#include "app_common.h"
#include "example_code.h"
#include "print_info.h"

/* Drivers */
static rtos_qspi_flash_t qspi_flash_ctx_s;
rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;

static rtos_l2_cache_t l2_cache_ctx_s;
rtos_l2_cache_t *l2_cache_ctx = &l2_cache_ctx_s;

/* 1 for direct, 0 for two way associative */
#define DIRECT_MAP 0

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
}

#if DIRECT_MAP
__attribute__((aligned(
        8))) static int l2_cache_buffer[RTOS_L2_CACHE_BUFFER_WORDS_DIRECT_MAP];
#else
__attribute__((aligned(
        8))) static int l2_cache_buffer[RTOS_L2_CACHE_BUFFER_WORDS_TWO_WAY];
#endif /* DIRECT_MAP */

L2_CACHE_SWMEM_READ_FN
void rtos_flash_read_wrapper(void *dst_address, const void *src_address,
                             const unsigned bytes)
{
    // rtos_printf("flash read dst: %p, src: %p, %d bytes\n", dst_address, src_address, bytes);
    int ret = -1;
    do {
        ret = rtos_qspi_flash_read_ll(qspi_flash_ctx, (uint8_t *)dst_address,
                                      (unsigned)(src_address - XS1_SWMEM_BASE),
                                      (size_t)bytes);
    } while (ret != 0);
}

void app(void *args)
{
    rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES - 1);
    rtos_l2_cache_start(l2_cache_ctx);

    while (1) {
        rtos_printf("Run examples\n");

        //   code execution
        example_code();

        //   code execution again (this time the code should already be cached)
        example_code();

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c1;
    (void)c2;
    (void)c3;

    /** The following configuration is only required for the Explorer Board 2v0
     ** due to an incorrect sfdp parameters being reported for the particular
     ** flash component sourced. **/
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

    rtos_qspi_flash_init(qspi_flash_ctx, XS1_CLKBLK_1, PORT_SQI_CS,
                         PORT_SQI_SCLK, PORT_SQI_SIO,

                         /** Derive QSPI clock from the 600 MHz xcore clock **/
                         qspi_io_source_clock_xcore,

                         /** Full speed clock configuration **/
                         5, // 600 MHz / (2*5) -> 60 MHz,
                         1, qspi_io_sample_edge_rising, 0,

                         /** SPI read clock configuration **/
                         12, // 600 MHz / (2*12) -> 25 MHz
                         0, qspi_io_sample_edge_falling, 0,

                         qspi_flash_page_program_1_4_4);

    rtos_l2_cache_init(l2_cache_ctx,
#if DIRECT_MAP
                       RTOS_L2_CACHE_DIRECT_MAP,
#else
                       RTOS_L2_CACHE_TWO_WAY_ASSOCIATIVE,
#endif /* DIRECT_MAP */
                       rtos_flash_read_wrapper, 1 << 1, /* Place on core 1 */
                       l2_cache_buffer);

    xTaskCreate((TaskFunction_t)app, "app", RTOS_THREAD_STACK_SIZE(app), NULL,
                configMAX_PRIORITIES - 1, NULL);

    rtos_printf("start scheduler on tile 0\n");

    vTaskStartScheduler();
    return;
}
