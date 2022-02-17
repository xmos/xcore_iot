// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* Library headers */
#include "rtos_osal.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/qspi_flash/qspi_flash_test.h"

static const char* test_name = "read_write_read_test";

#define local_printf( FMT, ... )    qspi_flash_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define QSPI_FLASH_TILE         0
#define QSPI_FLASH_TEST_ADDR    0

#if ON_TILE(QSPI_FLASH_TILE)
static int read_write_read(rtos_qspi_flash_t *ctx, unsigned addr, size_t test_len)
{
    local_printf("Test addr 0x%x len %u", addr, test_len);
    uint8_t *test_buf = NULL;

    test_buf = (uint8_t*)rtos_osal_malloc(test_len * sizeof(uint8_t));

    if (test_buf == NULL)
    {
        local_printf("Malloc Failed");
        return -1;
    }

    /* Erase sector and verify */
    local_printf("Erase");
    rtos_qspi_flash_erase(ctx, addr, test_len);
    local_printf("Read");
    rtos_qspi_flash_read(ctx, test_buf, addr, test_len);

    local_printf("Verify");
    for (int i=0; i<test_len; i++)
    {
        if (test_buf[i] != 0xFF)
        {
            local_printf("Failed. rx_buf[%d]: Expected 0 got 0x%x", i, test_buf[i]);
            rtos_osal_free(test_buf);
            return -1;
        }
    }

    /* Write sector */
    for (int i=0; i<test_len; i++)
    {
        test_buf[i] = i%(sizeof(uint8_t));
    }
    local_printf("Write");
    rtos_qspi_flash_write(ctx, test_buf, addr, test_len);
    memset(test_buf, 0, test_len);
    local_printf("Read");
    rtos_qspi_flash_read(ctx, test_buf, addr, test_len);

    local_printf("Verify");
    for (int i=0; i<test_len; i++)
    {
        if (test_buf[i] != (i%(sizeof(uint8_t))))
        {
            local_printf("Failed. rx_buf[%d]: Expected %d got 0x%x", i, i%(sizeof(uint8_t)), test_buf[i]);
            rtos_osal_free(test_buf);
            return -1;
        }
    }

    rtos_osal_free(test_buf);
    return 0;
}
#endif

QSPI_FLASH_MAIN_TEST_ATTR
static int main_test(qspi_flash_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(QSPI_FLASH_TILE)
    {
        size_t sector_size = rtos_qspi_flash_sector_size_get(ctx->qspi_flash_ctx);

        for (unsigned addr=0; addr<0x100; addr+=0x20)
        {
            if (read_write_read(ctx->qspi_flash_ctx, addr, sector_size) == -1)
            {
                return -1;
            }

            if (read_write_read(ctx->qspi_flash_ctx, addr, sector_size << 1) == -1)
            {
                return -1;
            }

            if (read_write_read(ctx->qspi_flash_ctx, addr, sector_size >> 1) == -1)
            {
                return -1;
            }

            if (read_write_read(ctx->qspi_flash_ctx, addr, 131072) == -1)
            {
                return -1;
            }
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

void register_read_write_read_test(qspi_flash_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
