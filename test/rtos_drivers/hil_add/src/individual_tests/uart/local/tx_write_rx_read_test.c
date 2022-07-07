// Copyright 22022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/uart/uart_test.h"

#if ON_TILE(UART_RX_TILE)
static volatile int error_cb_occurred = 0;

UART_RX_STARTED_ATTR
void rx_started(rtos_uart_rx_t *ctx){
    uart_printf("rtos_uart_rx_started_cb");
}

UART_RX_ERROR_ATTR
void rx_error(rtos_uart_rx_t *ctx, uint8_t err_flags){

    uart_printf("rtos_uart_rx_error:");
    if(err_flags & UR_START_BIT_ERR_CB_FLAG){
        uart_printf("UART_START_BIT_ERROR");
    }
    if(err_flags & UR_PARITY_ERR_CB_FLAG){
        uart_printf("UART_PARITY_ERROR");
    }
    if(err_flags & UR_FRAMING_ERR_CB_FLAG){
        uart_printf("UART_FRAMING_ERROR");
    }
    if(err_flags & UR_OVERRUN_ERR_CB_FLAG){
        uart_printf("UR_OVERRUN_ERR_CB_CODE");
    }

    if(err_flags & ~ RX_ERROR_FLAGS){
        uart_printf("UNKNOWN ERROR FLAG SET: 0x%x (THIS SHOULD NEVER HAPPEN)", err_flags);
    }

    error_cb_occurred = 1;
}

UART_RX_COMPLETE_ATTR
void rx_complete(rtos_uart_rx_t *ctx){
    // uart_printf("UART_RX_COMPLETE\n");
    // The backpressure from this causes a test fail - WHY?
}
#endif

static const char* test_name = "uart_loopback_test";

UART_MAIN_TEST_ATTR
static int main_test(uart_test_ctx_t *ctx)
{
    int retval = 0;
    const unsigned num_packets = 100;
    const uint8_t tx_buff[] = {0xed, 0x00, 0x77, 0xed, 0x00, 0x77, 0xed, 0x00, 0x55, 0x55, 0xff, 0x55,
                               0xed, 0x00, 0x77, 0xed, 0x00, 0x77, 0xed, 0x00, 0x55, 0x55, 0xff, 0x55,
                               0xed, 0x00, 0x77, 0xed, 0x00, 0x77, 0xed, 0x00, 0x55, 0x55, 0xff, 0x55,
                               0xed, 0x00, 0x77, 0xed, 0x00, 0x77, 0xed, 0x00, 0x55, 0x55, 0xff, 0x55,
                               0xed, 0x00, 0x77, 0xed, 0x00, 0x77, 0xed, 0x00, 0x55, 0x55, 0xff, 0x55 };//6 x 12 = 72B

#if ON_TILE(UART_TX_TILE)
    for(unsigned t = 0; t < num_packets; t++)
    {
        rtos_uart_tx_write(ctx->rtos_uart_tx_ctx, tx_buff, sizeof(tx_buff));
        vTaskDelay(pdMS_TO_TICKS(1));
    }
#endif

#if ON_TILE(UART_RX_TILE)
    for(unsigned t = 0; t < num_packets; t++)
    {
        uint8_t rx_buff[sizeof(tx_buff)] = {0};
        memset(rx_buff, 0x11, sizeof(rx_buff));

        size_t num_read_tot = 0;
        size_t num_timeouts = 0;
        while(num_read_tot < sizeof(rx_buff) && num_timeouts == 0){
            size_t num_rx = xStreamBufferReceive(ctx->rtos_uart_rx_ctx->app_byte_buffer,
                                                &rx_buff[num_read_tot],
                                                sizeof(rx_buff),
                                                pdMS_TO_TICKS(100));
            num_read_tot += num_rx;
            num_timeouts += (num_rx == 0);
        }

        int length_same = (num_read_tot == sizeof(tx_buff));
        int array_different = memcmp(tx_buff, rx_buff, sizeof(tx_buff));
        if(!length_same || array_different){
            uart_printf("uart loopback result len: %s, contents: %s", length_same ? "PASS" : "FAIL", array_different ? "FAIL" : "PASS");
        }
        if(!length_same){
            uart_printf("len expected: %d, actual: %d", sizeof(tx_buff), num_read_tot);
            retval = -1;
        }
        if(array_different){
            for(int i = 0; i < sizeof(tx_buff); i++){
                if(rx_buff[i] != tx_buff[i]){
                    uart_printf("Rx byte %d got: 0x%x exp: 0x%x", i, rx_buff[i], tx_buff[i]);
                }
            }
            retval = -1;
        }
    }

    if(error_cb_occurred){
        retval = -1;
        error_cb_occurred = 0;
        ctx->rx_success[ctx->cur_test] = 0;
    }

    if (ctx->rx_success[ctx->cur_test] != 0)
    {
        uart_printf("RX failed");
    }
#endif

    return retval;
}

void register_local_loopback_test(uart_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    uart_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

#if ON_TILE(UART_RX_TILE)
    test_ctx->uart_rx_started[this_test_num] = rx_started;
    test_ctx->uart_rx_error[this_test_num] = rx_error;
    test_ctx->uart_rx_complete[this_test_num] = rx_complete;
#endif

#if ON_TILE(UART_TX_TILE)
    test_ctx->uart_rx_started[this_test_num] = NULL;
    test_ctx->uart_rx_error[this_test_num] = NULL;
    test_ctx->uart_rx_complete[this_test_num] = NULL;
#endif

    test_ctx->rx_success[this_test_num] = 0;

    test_ctx->test_cnt++;
}
