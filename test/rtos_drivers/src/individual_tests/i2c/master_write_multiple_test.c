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

#define local_printf( FMT, ... )    i2c_printf("master_write_multiple_test|" FMT, ##__VA_ARGS__)

#define I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE   4
static uint8_t write_multiple_test_vector[I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE] =
{
    0xDE, 0xAD, 0xBE, 0xEF,
};

#if ON_TILE(1)
static uint32_t test_slave_iters = 0;

int master_write_multiple_test_i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    local_printf("SLAVE read");

    size_t expected_len = (test_slave_iters == 0) ? 1 : (I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE-1);
    uint8_t *ptr = (test_slave_iters == 0) ? &write_multiple_test_vector[0] : &write_multiple_test_vector[1];
    if (len != expected_len)
    {
        local_printf("SLAVE failed got len %d expected %d", len, expected_len);
    } else {
        for (int i=0; i<expected_len; i++)
        {
            if (*(ptr+i) != *(data+i))
            {
                local_printf("SLAVE failed on byte %d got 0x%x expected 0x%x", i, *(data+i), write_multiple_test_vector[i]);
                break;
            }
        }
    }

    test_slave_iters++;
    return 1;
}
#endif

int master_write_multiple_test_master(rtos_i2c_master_t *i2c_master_ctx)
{
    local_printf("Start");

    #if ON_TILE(0)
    {
        i2c_res_t ret;
        size_t sent = 0;
        local_printf("MASTER write");

        ret = rtos_i2c_master_write(i2c_master_ctx, I2C_SLAVE_ADDR, (unsigned char*)&write_multiple_test_vector, 1, &sent, 0);
        if ((ret != I2C_ACK) || (sent != 1))
        {
            local_printf("MASTER write failed to send 1");
            return -1;
        }

        ret = rtos_i2c_master_write(i2c_master_ctx, I2C_SLAVE_ADDR, (unsigned char*)&write_multiple_test_vector[1], I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE-1, &sent, 0);
        if ((ret != I2C_ACK) || (sent != I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE-1))
        {
            local_printf("MASTER write failed to send %d", I2C_MASTER_WRITE_MULTIPLE_TEST_SIZE-1);
            return -1;
        }
        rtos_i2c_master_stop_bit_send(i2c_master_ctx);
        local_printf("MASTER sent stop bit");
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
