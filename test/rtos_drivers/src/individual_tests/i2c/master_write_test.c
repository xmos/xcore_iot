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

#define local_printf( FMT, ... )    i2c_printf("master_write_test|" FMT, ##__VA_ARGS__)

#define I2C_MASTER_WRITE_TEST_SIZE   4
static uint8_t write_test_vector[I2C_MASTER_WRITE_TEST_SIZE] =
{
    0x00, 0xFF, 0xAA, 0x55,
};

#if ON_TILE(1)
static uint32_t test_slave_iters = 0;

int master_write_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read");
    if (len != I2C_MASTER_WRITE_TEST_SIZE)
    {
        local_printf("SLAVE failed got len %d expected %d", test_slave_iters, len, I2C_MASTER_WRITE_TEST_SIZE);
    } else {
        for (int i=0; i<I2C_MASTER_WRITE_TEST_SIZE; i++)
        {
            if (write_test_vector[i] != *(data+i))
            {
                local_printf("SLAVE failed on byte %d got 0x%x expected 0x%x", i, *(data+i), write_test_vector[i]);
                break;
            }
        }
    }

    test_slave_iters++;
    return 1;
}
#endif

int master_write_test_master(rtos_i2c_master_t *i2c_master_ctx)
{
    local_printf("Start");

    #if ON_TILE(0)
    {
        i2c_res_t ret;
        size_t sent = 0;
        local_printf("MASTER write");
        ret = rtos_i2c_master_write(i2c_master_ctx, I2C_SLAVE_ADDR, (unsigned char*)&write_test_vector, I2C_MASTER_WRITE_TEST_SIZE, &sent, 1);
        if (ret != I2C_ACK)
        {
            local_printf("MASTER failed");
            return -1;
        }
        if (sent != I2C_MASTER_WRITE_TEST_SIZE)
        {
            local_printf("MASTER failed sent %d expected %d", sent, I2C_MASTER_WRITE_TEST_SIZE);
            return -1;
        }
    }
    #endif

    #if ON_TILE(1)
    {
        while(test_slave_iters < 1)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#undef local_printf
