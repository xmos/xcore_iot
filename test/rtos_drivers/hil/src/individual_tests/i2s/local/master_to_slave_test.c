// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_i2s.h"

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

    #if ON_TILE(I2S_MASTER_TILE)
    {
        size_t tx_len = 0;
        int32_t tx_buf[I2S_FRAME_LEN*FRAME_NUM_CHANS] = {0};

        tx_buf[0] = 0x01234567;
        tx_buf[1] = 0x76543210;
        for (int i=2; i<(I2S_FRAME_LEN*FRAME_NUM_CHANS); i+=2)
        {
            tx_buf[i] = i;
            tx_buf[i+1] = i;
        }

        local_printf("MASTER tx");
        tx_len = rtos_i2s_tx(ctx->i2s_master_ctx,
                             tx_buf,
                             I2S_FRAME_LEN,
                             portMAX_DELAY);
        if (tx_len != I2S_FRAME_LEN)
        {
            local_printf("MASTER failed during tx.  Got %d expected %d", tx_len, I2S_FRAME_LEN);
            return -1;
        }
    }
    #endif

    #if ON_TILE(I2S_SLAVE_TILE)
    {
        int32_t rx_buf[I2S_FRAME_LEN*FRAME_NUM_CHANS] = {0};
        size_t rx_len = 0;
        int start = 0;
        do
        {
            rx_len = rtos_i2s_rx(ctx->i2s_slave_ctx,
                                 rx_buf,
                                 1,
                                 portMAX_DELAY);
             if (!start)
             {
                 if ((rx_buf[0] == 0x01234567) && (rx_buf[1] == 0x76543210))
                 {
                     local_printf("SLAVE start");
                     start = 1;
                 }
             } else {
                 if ((rx_buf[0] != (start << 1)) || (rx_buf[1] != (start << 1)))
                 {
                     local_printf("SLAVE failed on %d got %d %d expected %d", start, rx_buf[0], rx_buf[1], start <<1);
                     return -1;
                 }
                 start++;
             }
        } while (start < I2S_FRAME_LEN);
    }
    #endif

    local_printf("Done");
    return 0;
}

void register_master_to_slave_test(i2s_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
