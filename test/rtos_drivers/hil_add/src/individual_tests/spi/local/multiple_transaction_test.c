// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_spi_master.h"
#include "rtos_spi_slave.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/spi/spi_test.h"

static const char* test_name = "multiple_transaction_test";

#define local_printf( FMT, ... )    spi_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define SPI_MASTER_TILE 0
#define SPI_SLAVE_TILE  1

#if ON_TILE(SPI_MASTER_TILE) || ON_TILE(SPI_SLAVE_TILE)
static uint8_t test_buf[SPI_TEST_BUF_SIZE] = {0};
#endif

#if ON_TILE(SPI_SLAVE_TILE)
/* Since callbacks aren't used for data transmission, just verify that they
 * were called */
static uint32_t xfer_done_called = 0;
#endif

SPI_MAIN_TEST_ATTR
static int main_test(spi_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(SPI_SLAVE_TILE)
    {
        uint8_t in_buf[SPI_TEST_BUF_SIZE] = {0};
        local_printf("SLAVE transaction");

        spi_slave_xfer_prepare(
                ctx->spi_slave_ctx,
                in_buf,
                SPI_TEST_BUF_SIZE,
                test_buf,
                SPI_TEST_BUF_SIZE);
    }
    #endif

    #if ON_TILE(SPI_MASTER_TILE)
    {
        uint8_t in_buf[SPI_TEST_BUF_SIZE] = {0};
        local_printf("MASTER transaction");

        rtos_spi_master_delay_before_next_transfer(ctx->spi_device_ctx, 1000);

        rtos_spi_master_transaction_start(ctx->spi_device_ctx);
        rtos_spi_master_transfer(ctx->spi_device_ctx, test_buf, in_buf, 1);
        rtos_spi_master_transfer(ctx->spi_device_ctx, test_buf+1, in_buf+1, SPI_TEST_BUF_SIZE-1);
        rtos_spi_master_transaction_end(ctx->spi_device_ctx);

        for (int i=0; i<SPI_TEST_BUF_SIZE; i++)
        {
            if (in_buf[i] != test_buf[i])
            {
                local_printf("MASTER failed on iteration %d got 0x%x expected 0x%x", i, in_buf[i], test_buf[i]);
                return -1;
            }
        }
    }
    #endif

    #if ON_TILE(SPI_SLAVE_TILE)
    {
        uint8_t *rx_buf = NULL;
        size_t rx_len = 0;
        uint8_t *tx_buf = NULL;
        size_t tx_len = 0;

        int ret = spi_slave_xfer_complete(
                ctx->spi_slave_ctx,
                (void**)&rx_buf,
                &rx_len,
                (void**)&tx_buf,
                &tx_len,
                pdMS_TO_TICKS(10000));

        if (ret != 0)
        {
            local_printf("SLAVE failed. Transfer timed out");
            return -1;
        }

        if (rx_len != SPI_TEST_BUF_SIZE) {
            local_printf("SLAVE failed. RX len got %u expected %u", rx_len, SPI_TEST_BUF_SIZE);
            return -1;
        } else if (tx_len != SPI_TEST_BUF_SIZE) {
            local_printf("SLAVE failed. TX len got %u expected %u", tx_len, SPI_TEST_BUF_SIZE);
            return -1;
        }

        if (rx_buf == NULL) {
            local_printf("SLAVE failed. rx_buf is NULL");
            return -1;
        } else if (tx_buf == NULL) {
            local_printf("SLAVE failed. tx_buf is NULL");
            return -1;
        }

        for (int i=0; i<SPI_TEST_BUF_SIZE; i++)
        {
            if (rx_buf[i] != test_buf[i]) {
                local_printf("SLAVE failed. rx_buf[%d] got 0x%x expected 0x%x", i, rx_buf[i], test_buf[i]);
                return -1;
            } else if (tx_buf[i] != test_buf[i]) {
                local_printf("SLAVE failed. tx_buf[%d] got 0x%x expected 0x%x", i, tx_buf[i], test_buf[i]);
                return -1;
            }
        }

        if (xfer_done_called != 1) {
            local_printf("SLAVE failed. slave_xfer_done callback did not occur");
            return -1;
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#if ON_TILE(SPI_SLAVE_TILE)
SPI_SLAVE_XFER_DONE_ATTR
static int slave_xfer_done(rtos_spi_slave_t *ctx, void *app_data)
{
    local_printf("SLAVE slave_xfer_done");
    xfer_done_called = 1;
    return 0;
}

#endif

void register_multiple_transaction_test(spi_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

#if ON_TILE(SPI_MASTER_TILE) || ON_TILE(SPI_SLAVE_TILE)
    for (int i=0; i<SPI_TEST_BUF_SIZE; i++)
    {
        test_buf[i] = (uint8_t)(i % sizeof(uint8_t));
    }
#endif

#if ON_TILE(SPI_SLAVE_TILE)
    xfer_done_called = 0;
    test_ctx->slave_xfer_done[this_test_num] = slave_xfer_done;
#endif

    test_ctx->test_cnt++;
}

#undef local_printf
