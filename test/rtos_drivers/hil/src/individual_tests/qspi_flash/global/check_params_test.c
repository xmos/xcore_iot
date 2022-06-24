// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* Library headers */
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/qspi_flash/qspi_flash_test.h"

static const char* test_name = "check_params_test";

#define local_printf( FMT, ... )    qspi_flash_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define QSPI_FLASH_FLASH_PART_SIZE          8388608
#define QSPI_FLASH_FLASH_PART_PAGE_SIZE     256
#define QSPI_FLASH_FLASH_PART_PAGE_CNT      32768
#define QSPI_FLASH_FLASH_SECTOR_SIZE        4096

QSPI_FLASH_MAIN_TEST_ATTR
static int main_test(qspi_flash_test_ctx_t *ctx)
{
    local_printf("Start");

    size_t flash_size = rtos_qspi_flash_size_get(ctx->qspi_flash_ctx);
    size_t page_size = rtos_qspi_flash_page_size_get(ctx->qspi_flash_ctx);
    size_t page_cnt = rtos_qspi_flash_page_count_get(ctx->qspi_flash_ctx);
    size_t sector_size = rtos_qspi_flash_sector_size_get(ctx->qspi_flash_ctx);

    if (flash_size != QSPI_FLASH_FLASH_PART_SIZE)
    {
        local_printf("Failed flash part size incorrect.  Got %u expected %u", flash_size, QSPI_FLASH_FLASH_PART_SIZE);
        return -1;
    } else {
        local_printf("Flash part size correct");
    }

    if (page_size != QSPI_FLASH_FLASH_PART_PAGE_SIZE)
    {
        local_printf("Failed flash part page size incorrect.  Got %u expected %u", page_size, QSPI_FLASH_FLASH_PART_PAGE_SIZE);
        return -1;
    } else {
        local_printf("Flash part page size correct");
    }

    if (page_cnt != QSPI_FLASH_FLASH_PART_PAGE_CNT)
    {
        local_printf("Failed flash part count incorrect.  Got %u expected %u", page_cnt, QSPI_FLASH_FLASH_PART_PAGE_CNT);
        return -1;
    } else {
        local_printf("Flash part page count correct");
    }

    if (sector_size != QSPI_FLASH_FLASH_SECTOR_SIZE)
    {
        local_printf("Failed flash part sector size incorrect.  Got %u expected %u", sector_size, QSPI_FLASH_FLASH_SECTOR_SIZE);
        return -1;
    } else {
        local_printf("Flash part sector size correct");
    }

    local_printf("Done");
    return 0;
}

void register_check_params_test(qspi_flash_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
