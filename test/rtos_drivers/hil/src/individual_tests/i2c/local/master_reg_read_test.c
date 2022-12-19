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

static const char* test_name = "master_reg_read_test";

#define local_printf( FMT, ... )    i2c_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define I2C_MASTER_TILE 0
#define I2C_SLAVE_TILE  1

typedef struct reg_test {
    uint8_t reg;
    uint8_t val;
} reg_test_t;

#if ON_TILE(I2C_MASTER_TILE) || ON_TILE(I2C_SLAVE_TILE)
#define I2C_MASTER_REG_READ_TEST_ITER   4
static reg_test_t test_vector[I2C_MASTER_REG_READ_TEST_ITER] =
{
    {0xDE, 0xFF},
    {0xAD, 0x00},
    {0xBE, 0xAA},
    {0xEF, 0x55},
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
        i2c_regop_res_t reg_ret;
        for (int i=0; i<I2C_MASTER_REG_READ_TEST_ITER; i++)
        {
            uint8_t tmpval = 0;
            local_printf("MASTER read iteration %d", i);
            reg_ret = rtos_i2c_master_reg_read(ctx->i2c_master_ctx,
                                               I2C_SLAVE_ADDR,
                                               test_vector[i].reg,
                                               &tmpval);

            if (reg_ret != I2C_REGOP_SUCCESS)
            {
                local_printf("MASTER failed on iteration %d", i);
                return -1;
            }

            if (tmpval != test_vector[i].val)
            {
                local_printf("MASTER failed on iteration %d got 0x%x expected 0x%x", i, tmpval, test_vector[i].val);
                return -1;
            }
        }
    }
    #endif

    #if ON_TILE(I2C_SLAVE_TILE)
    {
        while(test_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
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
static uint8_t test_slave_send_val = 0;
static uint8_t* test_slave_send_val_ptr = &test_slave_send_val;

I2C_SLAVE_RX_ATTR
static void slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read iteration %d", test_slave_iters);
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;

    if (len != 1)
    {
        local_printf("SLAVE failed on iteration %d got len %d expected %d", test_slave_iters, len, 1);
        test_ctx->slave_success[test_ctx->cur_test] = -1;
    } else {
        for (int i=0; i<I2C_MASTER_REG_READ_TEST_ITER ;i++)
        {
            if (test_vector[i].reg == *data)
            {
                test_slave_send_val = test_vector[i].val;
                local_printf("SLAVE rx set tx val to 0x%x", test_slave_send_val);
                break;
            }
        }
    }

    test_slave_iters++;
}

I2C_SLAVE_TX_START_ATTR
static size_t slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    size_t len = 0;

    *data = test_slave_send_val_ptr;
    local_printf("SLAVE tx 0x%x", **data);
    len = 1;

    return len;
}

I2C_SLAVE_TX_DONE_ATTR
static void slave_tx_done(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE tx done %d bytes", len);

    if (len != 1)
    {
        i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
        test_ctx->slave_success[test_ctx->cur_test] = -1;
    }
}

I2C_SLAVE_RX_BYTE_CHECK_ATTR
static void slave_rx_byte_check(rtos_i2c_slave_t *ctx, void *app_data, uint8_t data, i2c_slave_ack_t *cur_status)
{
    local_printf("SLAVE rx byte check 0x%x status %d", data, *cur_status);
}
#endif

void register_master_reg_read_test(i2c_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    #if ON_TILE(I2C_SLAVE_TILE)
    test_ctx->slave_rx[this_test_num] = slave_rx;
    test_ctx->slave_tx_start[this_test_num] = slave_tx_start;
    test_ctx->slave_tx_done[this_test_num] = slave_tx_done;
    test_ctx->slave_rx_check_byte[this_test_num] = slave_rx_byte_check;
    test_ctx->slave_write_addr_req[this_test_num] = NULL;
    #endif

    #if ON_TILE(I2C_MASTER_TILE)
    test_ctx->slave_rx[this_test_num] = NULL;
    #endif

    test_ctx->test_cnt++;
}

#undef local_printf
