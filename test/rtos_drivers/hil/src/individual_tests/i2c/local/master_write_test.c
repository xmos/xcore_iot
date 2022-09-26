// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/i2c/i2c_test.h"

static const char* test_name = "master_write_test";

#define local_printf( FMT, ... )    i2c_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define I2C_MASTER_TILE 0
#define I2C_SLAVE_TILE  1

#if ON_TILE(I2C_MASTER_TILE) || ON_TILE(I2C_SLAVE_TILE)
#define I2C_MASTER_WRITE_TEST_ITER   2
#define I2C_MASTER_WRITE_TEST_SIZE   4
static uint8_t test_vector[I2C_MASTER_WRITE_TEST_ITER][I2C_MASTER_WRITE_TEST_SIZE] =
{
    {0x00, 0xFF, 0xAA, 0x55},
    {0xDE, 0xAD, 0xBE, 0xEF},
};
#endif

#if ON_TILE(I2C_SLAVE_TILE)
static uint32_t test_slave_iters = 0;
#endif

I2C_MAIN_TEST_ATTR
static int main_test(i2c_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(I2C_MASTER_TILE)
    {
        i2c_res_t ret;
        for (int i=0; i<I2C_MASTER_WRITE_TEST_ITER; i++)
        {
            size_t sent = 0;
            local_printf("MASTER write iteration %d", i);
            ret = rtos_i2c_master_write(ctx->i2c_master_ctx, I2C_SLAVE_ADDR, (unsigned char*)&test_vector[i], I2C_MASTER_WRITE_TEST_SIZE, &sent, 1);
            if (ret != I2C_ACK)
            {
                local_printf("MASTER failed");
                return -1;
            }
            if (sent != I2C_MASTER_WRITE_TEST_SIZE)
            {
                local_printf("MASTER failed on iteration %d sent %d expected %d", i, sent, I2C_MASTER_WRITE_TEST_SIZE);
                return -1;
            }
        }
    }
    #endif

    #if ON_TILE(I2C_SLAVE_TILE)
    {
        while(test_slave_iters < I2C_MASTER_WRITE_TEST_ITER)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        if (ctx->slave_success[ctx->cur_test] != 0)
        {
            local_printf("SLAVE failed");
            return -1;
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#if ON_TILE(I2C_SLAVE_TILE)
I2C_SLAVE_RX_ATTR
static void slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read iteration %d", test_slave_iters);
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;

    if (len != I2C_MASTER_WRITE_TEST_SIZE)
    {
        local_printf("SLAVE failed got len %d expected %d", test_slave_iters, len, I2C_MASTER_WRITE_TEST_SIZE);
    } else {
        for (int i=0; i<I2C_MASTER_WRITE_TEST_SIZE; i++)
        {
            if (test_vector[test_slave_iters][i] != *(data+i))
            {
                local_printf("SLAVE failed on byte %d got 0x%x expected 0x%x", i, *(data+i), test_vector[test_slave_iters][i]);
                test_ctx->slave_success[test_ctx->cur_test] = -1;
                break;
            }
        }
    }

    test_slave_iters++;
}

static int slave_byte_check = 0;

I2C_SLAVE_RX_BYTE_CHECK_ATTR
static void slave_rx_byte_check(rtos_i2c_slave_t *ctx, void *app_data, uint8_t data, i2c_slave_ack_t *cur_status)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    local_printf("SLAVE rx byte check 0x%x status %d", data, *cur_status);
    int i = slave_byte_check % I2C_MASTER_WRITE_TEST_SIZE;
    slave_byte_check++;
    if (test_vector[test_slave_iters][i] != data) {
        local_printf("SLAVE rx byte check failed");
        test_ctx->slave_success[test_ctx->cur_test] = -1;
    }
}
#endif

void register_master_write_test(i2c_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    #if ON_TILE(I2C_SLAVE_TILE)
    test_ctx->slave_rx[this_test_num] = slave_rx;
    test_ctx->slave_rx_check_byte[this_test_num] = slave_rx_byte_check;
    test_ctx->slave_write_addr_req[this_test_num] = NULL;
    #endif

    #if ON_TILE(I2C_MASTER_TILE)
    test_ctx->slave_rx[this_test_num] = NULL;
    #endif

    test_ctx->slave_tx_start[this_test_num] = NULL;
    test_ctx->slave_tx_done[this_test_num] = NULL;

    test_ctx->test_cnt++;
}

#undef local_printf
