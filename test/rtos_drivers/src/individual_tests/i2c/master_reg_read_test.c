// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/i2c/api/rtos_i2c_slave.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/i2c/i2c_test.h"

#define local_printf( FMT, ... )    i2c_printf("master_reg_read_test|" FMT, ##__VA_ARGS__)

typedef struct reg_test {
    uint8_t reg;
    uint8_t val;
} reg_test_t;

#define I2C_MASTER_REG_READ_TEST_ITER   4
static reg_test_t read_reg_test_vector[I2C_MASTER_REG_READ_TEST_ITER] =
{
    {0xDE, 0xFF},
    {0xAD, 0x00},
    {0xBE, 0xAA},
    {0xEF, 0x55},
};

#if ON_TILE(1)
static uint32_t test_slave_iters = 0;
static uint8_t test_slave_send_val = 0;
static uint8_t* test_slave_send_val_ptr = &test_slave_send_val;

int master_reg_read_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read iteration %d", test_slave_iters);

    if (len != 1)
    {
        local_printf("SLAVE failed on iteration %d got len %d expected %d", test_slave_iters, len, 1);
    } else {
        for (int i=0; i<I2C_MASTER_REG_READ_TEST_ITER ;i++)
        {
            if (read_reg_test_vector[i].reg == *data)
            {
                test_slave_send_val = read_reg_test_vector[i].val;
                local_printf("SLAVE rx set tx val to 0x%x", test_slave_send_val);
                break;
            }
        }
    }

    test_slave_iters++;
    return (test_slave_iters == I2C_MASTER_REG_READ_TEST_ITER);
}

size_t master_reg_read_test_i2c_slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    size_t len = 0;

    *data = test_slave_send_val_ptr;
    local_printf("SLAVE tx 0x%x", **data);
    len = 1;

    if(test_slave_iters == I2C_MASTER_REG_READ_TEST_ITER)
    {
        i2c_slave_test_stage_increment();
    }

    return len;
}

#endif

int master_reg_read_test_master(rtos_i2c_master_t *i2c_master_ctx)
{
    local_printf("Start");

    #if ON_TILE(0)
    {
        i2c_regop_res_t reg_ret;
        for (int i=0; i<I2C_MASTER_REG_READ_TEST_ITER; i++)
        {
            uint8_t tmpval = 0;
            local_printf("MASTER read iteration %d", i);
            reg_ret = rtos_i2c_master_reg_read(i2c_master_ctx,
                                               I2C_SLAVE_ADDR,
                                               read_reg_test_vector[i].reg,
                                               &tmpval);

            if (reg_ret != I2C_REGOP_SUCCESS)
            {
                local_printf("MASTER failed on iteration %d", i);
                return -1;
            }

            if (tmpval != read_reg_test_vector[i].val)
            {
                local_printf("MASTER failed on iteration %d got 0x%x expected 0x%x", i, tmpval, read_reg_test_vector[i].val);
                return -1;
            }
        }
    }
    #endif

    #if ON_TILE(1)
    {
        while(test_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#undef local_printf
