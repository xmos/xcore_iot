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

#define local_printf( FMT, ... )    i2c_printf("master_reg_write_test|" FMT, ##__VA_ARGS__)

typedef struct reg_test {
    uint8_t reg;
    uint8_t val;
} reg_test_t;

#define I2C_MASTER_REG_WRITE_TEST_ITER   4
static reg_test_t write_reg_test_vector[I2C_MASTER_REG_WRITE_TEST_ITER] =
{
    {0x00, 0xDE},
    {0xFF, 0xAD},
    {0xFA, 0xBE},
    {0xB4, 0xEF},
};

#if ON_TILE(1)
static uint32_t test_slave_iters = 0;

int master_reg_write_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read iteration %d", test_slave_iters);
    if (len != 2)
    {
        local_printf("SLAVE failed on iteration %d got len %d expected %d", test_slave_iters, len, 2);
    }
    if (write_reg_test_vector[test_slave_iters].reg != *data)
    {
        local_printf("SLAVE failed on iteration %d got reg 0x%x expected 0x%x", test_slave_iters, write_reg_test_vector[test_slave_iters].reg, *data);
    }
    if (write_reg_test_vector[test_slave_iters].val != *++data)
    {
        local_printf("SLAVE failed on iteration %d got val 0x%x expected 0x%x", test_slave_iters, write_reg_test_vector[test_slave_iters].val, *++data);
    }
    test_slave_iters++;
    return (test_slave_iters == I2C_MASTER_REG_WRITE_TEST_ITER);
}
#endif

int master_reg_write_test_master(rtos_i2c_master_t *i2c_master_ctx)
{
    local_printf("Start");

    #if ON_TILE(0)
    {
        i2c_regop_res_t reg_ret;
        for (int i=0; i<I2C_MASTER_REG_WRITE_TEST_ITER; i++)
        {
            local_printf("MASTER write iteration %d", i);
            reg_ret = rtos_i2c_master_reg_write(i2c_master_ctx,
                                                I2C_SLAVE_ADDR,
                                                write_reg_test_vector[i].reg,
                                                write_reg_test_vector[i].val);
            if (reg_ret != I2C_REGOP_SUCCESS)
            {
                local_printf("MASTER failed on iteration %d", i);
                return -1;
            }
        }
    }
    #endif

    #if ON_TILE(1)
    {
        while(test_slave_iters < I2C_MASTER_REG_WRITE_TEST_ITER)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#undef local_printf
