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

static const char* test_name = "slave_default_buffer_test";

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

static uint8_t default_in_buf[SPI_TEST_BUF_SIZE] = {0};
static size_t default_in_buf_len = SPI_TEST_BUF_SIZE;
static uint8_t default_out_buf[SPI_TEST_BUF_SIZE] = {0};
static size_t default_out_buf_len = SPI_TEST_BUF_SIZE;
#endif

SPI_MAIN_TEST_ATTR
static int main_test(spi_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(SPI_SLAVE_TILE)
    {
        uint8_t in_buf[SPI_TEST_BUF_SIZE] = {0};
        for(int i=0; i<SPI_TEST_BUF_SIZE; i++) {
            default_out_buf[i] = (uint8_t)i;
        }

        local_printf("SLAVE prepare default buffers");

        spi_slave_xfer_prepare_default_buffers(
                ctx->spi_slave_ctx,
                default_in_buf,
                SPI_TEST_BUF_SIZE,
                default_out_buf,
                SPI_TEST_BUF_SIZE);

        local_printf("SLAVE transaction");

        spi_slave_xfer_prepare(
                ctx->spi_slave_ctx,
                in_buf,
                SPI_TEST_BUF_SIZE,
                test_buf,
                SPI_TEST_BUF_SIZE);

        /* Setup handling response */
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

        /* First time it should be user provided buffer */
        if (!((rx_buf == in_buf) && tx_buf == test_buf)) {
            local_printf("SLAVE failed.  Unexpected buffer.  Expected user");
            return -1;
        }

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
            }
        }

        if (xfer_done_called != 1) {
            local_printf("SLAVE failed. slave_xfer_done callback did not occur");
            return -1;
        }

        ret = spi_slave_xfer_complete(
                ctx->spi_slave_ctx,
                (void**)&rx_buf,
                &rx_len,
                (void**)&tx_buf,
                &tx_len,
                pdMS_TO_TICKS(10000));

        /* Second time it should be default buffer */
        if (!((rx_buf == default_in_buf) && tx_buf == default_out_buf)) {
            local_printf("SLAVE failed.  Unexpected buffer.  Expected default");
            return -1;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    #endif

    #if ON_TILE(SPI_MASTER_TILE)
    {
        vTaskDelay(pdMS_TO_TICKS(100));

        uint8_t in_buf[SPI_TEST_BUF_SIZE] = {0};
        uint8_t in_buf_default[SPI_TEST_BUF_SIZE] = {0};

        /* Force a default buffer situation by the SPI slave */
        local_printf("MASTER transaction");
        rtos_spi_master_transaction_start(ctx->spi_device_ctx);
        rtos_spi_master_transfer(ctx->spi_device_ctx, test_buf, in_buf, SPI_TEST_BUF_SIZE);
        rtos_spi_master_transaction_end(ctx->spi_device_ctx);
        local_printf("MASTER transaction");
        rtos_spi_master_transaction_start(ctx->spi_device_ctx);
        rtos_spi_master_transfer(ctx->spi_device_ctx, test_buf, in_buf_default, SPI_TEST_BUF_SIZE);
        rtos_spi_master_transaction_end(ctx->spi_device_ctx);

        /* Verify that in_buf == test_buf, and in_buf_default == the default SPI Slave buffer */
        for (int i=0; i<SPI_TEST_BUF_SIZE; i++)
        {
            // local_printf("MASTER got in_buf[%d]:%d", i, in_buf[i]);

            if (in_buf[i] != test_buf[i])
            {
                local_printf("MASTER failed on iteration %d got 0x%x expected 0x%x", i, in_buf[i], test_buf[i]);
                return -1;
            }
        }
        for (int i=0; i<SPI_TEST_BUF_SIZE; i++)
        {
            // local_printf("MASTER got in_buf_default[%d]:%d", i, in_buf_default[i]);
            
            if (in_buf_default[i] != (uint8_t)i)
            {
                local_printf("MASTER failed on default buffer iteration %d got 0x%x expected 0x%x", i, in_buf_default[i], (uint8_t)i);
                return -1;
            }
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

void register_slave_default_buffer_test(spi_test_ctx_t *test_ctx)
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
