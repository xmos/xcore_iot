// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos/drivers/i2s/api/rtos_i2s.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/i2s/i2s_test.h"

static const char* test_name = "master_to_slave_test";

#define local_printf( FMT, ... )    i2s_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define I2S_MASTER_TILE 1
#define I2S_SLAVE_TILE  0

#define FRAME_NUM_CHANS 2

I2S_MAIN_TEST_ATTR
static int main_test(i2s_test_ctx_t *ctx)
{
    local_printf("Start");

    size_t rx_len = 0;
    int32_t rx_buf[I2S_FRAME_LEN*FRAME_NUM_CHANS] = {0};
    TaskHandle_t output_handle;

    #if ON_TILE(I2S_MASTER_TILE)
    {
        size_t tx_len = 0;
        int32_t tx_buf[I2S_FRAME_LEN*FRAME_NUM_CHANS] = {0};

        for (int i=0; i<(I2S_FRAME_LEN*FRAME_NUM_CHANS); i++)
        {
            tx_buf[i] = i;
        }
        while(1)
        {
            local_printf("MASTER tx");
            tx_len = rtos_i2s_tx(ctx->i2s_master_ctx,
                                 tx_buf,
                                 I2S_FRAME_LEN,
                                 portMAX_DELAY);
             vTaskDelay(pdMS_TO_TICKS(5000));
            // local_printf("MASTER txed %d",tx_len);
        }
        // xassert(tx_len == I2S_FRAME_LEN);
    }
    #endif

    #if ON_TILE(I2S_SLAVE_TILE)
    {
        while(1)
        {
            local_printf("SLAVE rx");
            rx_len = rtos_i2s_rx(ctx->i2s_slave_ctx,
                                 rx_buf,
                                 I2S_FRAME_LEN,
                                 portMAX_DELAY);

            // if (rx_len != I2S_FRAME_LEN)
            // {
            //     local_printf("SLAVE failed rx got %u expected %u", rx_len, I2S_FRAME_LEN);
            //     return -1;
            // }

            for (int i=0; i<I2S_FRAME_LEN*FRAME_NUM_CHANS; i++)
            {
                if (rx_buf[i] != (i << 1))
                {
                    local_printf("SLAVE failed got rx_buf[%d]:%u expected %u", i, rx_buf[i], i << 1);
                    // return -1;
                }
            }
        }
    }
    #endif

    local_printf("Done");
    return 0;
}

#if ON_TILE(I2S_SLAVE_TILE)
#endif

void register_master_to_slave_test(i2s_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
