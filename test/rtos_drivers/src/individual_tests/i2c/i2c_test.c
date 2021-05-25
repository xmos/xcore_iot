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

#if ON_TILE(1)
RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_start(rtos_i2c_slave_t *ctx, void *app_data)
{
    i2c_printf("SLAVE started");
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_rx[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_rx_t fn;
        fn = test_ctx->slave_rx[test_ctx->cur_test];
        fn(ctx, app_data, data, len);
    } else {
        i2c_printf("SLAVE missing slave_rx callback on test %d", test_ctx->cur_test);
    }
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static size_t i2c_slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    size_t len = 0;

    if (test_ctx->slave_tx_start[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_tx_start_t fn;
        fn = test_ctx->slave_tx_start[test_ctx->cur_test];
        len = fn(ctx, app_data, data);
    } else {
        i2c_printf("SLAVE missing i2c_slave_tx_start callback on test %d", test_ctx->cur_test);
    }

    return len;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_tx_done(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_tx_done[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_tx_done_t fn;
        fn = test_ctx->slave_tx_done[test_ctx->cur_test];
        fn(ctx, app_data, data, len);
    } else {
        i2c_printf("SLAVE missing i2c_slave_tx_done callback on test %d", test_ctx->cur_test);
    }
}
#endif /* ON_TILE(1) */

// #define I2C_MASTER_READ_TEST_SIZE   4
// static uint8_t read_test_vector[I2C_MASTER_READ_TEST_SIZE] =
// {
//     0xA5, 0x5A, 0x0F, 0xF0,
// };
//
// #define I2C_MASTER_READ_MULTIPLE_TEST_SIZE   4
// static uint8_t read_multiple_test_vector[I2C_MASTER_READ_MULTIPLE_TEST_SIZE] =
// {
//     0xDE, 0xAD, 0xBE, 0xEF,
// };

// #if ON_TILE(1)
// static uint32_t i2c_slave_test_stage = 0;
// static uint32_t test2_slave_iters = 0;
// static uint32_t test3_slave_iters = 0;
// static uint8_t test1_slave_send_val = 0;
// static uint8_t test3_slave_send_val = 0;
//
// void i2c_slave_test_stage_increment(void)
// {
//     i2c_slave_test_stage++;
// }
//
// void i2c_slave_start(rtos_i2c_slave_t *ctx, void *app_data)
// {
//     i2c_printf("Slave started");
// }
//
// void i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
// {
//     switch(i2c_slave_test_stage)
//     {
//         default:
//         {
//             i2c_printf("Slave unexpected test stage in i2c_slave_rx");
//             configASSERT(0);
//         }
//         break;
//
//         case 0:
//         {
//             if (master_reg_write_test_i2c_slave_rx(ctx, app_data, data, len) != 0)
//             {
//                 i2c_slave_test_stage_increment();
//             }
//         }
//         break;
//
//         case 1:
//         {
//             if (master_reg_read_test_i2c_slave_rx(ctx, app_data, data, len) != 0)
//             {
//                 ;   // test incremented after we request send
//             }
//         }
//         break;
//
//         case 2:
//         {
//             if (master_write_test_i2c_slave_rx(ctx, app_data, data, len) != 0)
//             {
//                 i2c_slave_test_stage_increment();
//             }
//         }
//         break;
//
//         case 3:
//         {
//             if (master_write_multiple_test_i2c_slave_rx(ctx, app_data, data, len) != 0)
//             {
//                 i2c_slave_test_stage_increment();
//             }
//         }
//         break;
//
//         // case 3:
//         // {
//         //     configASSERT(len == I2C_MASTER_WRITE_TEST_SIZE);
//         //     uint8_t *tmpptr = data;
//         //
//         //     for (int i=0; i< I2C_MASTER_WRITE_TEST_SIZE; i++)
//         //     {
//         //         if (write_test_vector[i] != *tmpptr)
//         //         {
//         //             i2c_printf("slave rx failed on iteration %d", i);
//         //             break;
//         //         }
//         //         tmpptr++;
//         //     }
//         // }
//         //
//         // static reg_test_t write_test_vector[I2C_MASTER_WRITE_TEST_SIZE]
//         // static reg_test_t write_multiple_test_vector[I2C_MASTER_WRITE_TEST_SIZE]
//         // static reg_test_t read_test_vector[I2C_MASTER_WRITE_TEST_SIZE]
//         // static reg_test_t read_multiple_test_vector[I2C_MASTER_READ_MULTIPLE_TEST_SIZE]
//     }
// }
//
// size_t i2c_slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
// {
//     size_t len = 0;
//
//     switch(i2c_slave_test_stage)
//     {
//         default:
//         {
//             i2c_printf("Slave unexpected test stage in i2c_slave_tx_start");
//             configASSERT(0);
//         }
//         break;
//
//         case 0:
//         {
//             i2c_printf("Slave tx 0");
//             len = 0;
//         }
//         break;
//
//         case 1:
//         {
//             len = master_reg_read_test_i2c_slave_tx_start(ctx, app_data, data);
//         }
//         break;
//
//         case 2:
//         {
//             i2c_printf("Slave tx 0");
//         }
//         break;
//
//         case 3:
//         {
//             i2c_printf("Slave tx 0");
//         }
//         break;
//     }
//
//     return len;
// }
//
// void i2c_slave_tx_done(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
// {
//     i2c_printf("Slave tx done len %d", len);
// }
// #endif /* ON_TILE(1) */
//
// void start_i2c_devices(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx)
// {
//     i2c_printf("Master rpc configure");
//     rtos_i2c_master_rpc_config(i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);
//
//     #if ON_TILE(0)
//     {
//         i2c_printf("Master start");
//         rtos_i2c_master_start(i2c_master_ctx);
//     }
//     #endif
//
//     #if ON_TILE(1)
//     {
//         i2c_printf("Slave start");
//         i2c_slave_test_stage = 0;
//         rtos_i2c_slave_start(i2c_slave_ctx,
//                              NULL,
//                              i2c_slave_start,
//                              i2c_slave_rx,
//                              i2c_slave_tx_start,
//                              i2c_slave_tx_done,
//                              I2C_SLAVE_ISR_CORE,
//                              configMAX_PRIORITIES-1);
//     }
//     #endif
//     i2c_printf("devices setup done");
// }
//
// int run_i2c_tests(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx)
// {
//     if (master_reg_write_test_master(i2c_master_ctx) != 0)
//     {
//         return -1;
//     }
//
//     if (master_reg_read_test_master(i2c_master_ctx) != 0)
//     {
//         return -1;
//     }
//
//     if (master_write_test_master(i2c_master_ctx) != 0)
//     {
//         return -1;
//     }
//
//     if (master_write_multiple_test_master(i2c_master_ctx) != 0)
//     {
//         return -1;
//     }
//
// #if 0
//     #if ON_TILE(0)
//     {
//         i2c_regop_res_t reg_ret;
//         i2c_res_t ret;
//
//
//         // i2c_printf("master write multiple stop bit test start");
//         // {
//         //     size_t sent = 0;
//         //     ret = rtos_i2c_master_write(i2c_master_ctx, I2C_SLAVE_ADDR, &write_multiple_test_vector, 1, &sent, 0);
//         //     if ((ret != I2C_ACK) || (sent != 1))
//         //     {
//         //         i2c_printf("master write multiple test failed on iteration");
//         //         return -1;
//         //     }
//         //
//         //     ret = rtos_i2c_master_write(i2c_master_ctx, I2C_SLAVE_ADDR, &write_multiple_test_vector[1], I2C_MASTER_WRITE_TEST_SIZE-1, &sent, 0);
//         //     if ((ret != I2C_ACK) || (sent != 3))
//         //     {
//         //         i2c_printf("master write multiple test failed on iteration");
//         //         return -1;
//         //     }
//         //     rtos_i2c_master_stop_bit_send(i2c_master_ctx);
//         // }
//         // i2c_printf("master write multiple stop bit test done");
//
//         i2c_printf("master read test start");
//         {
//             uint8_t read[I2C_MASTER_READ_TEST_SIZE] = {0};
//             ret = rtos_i2c_master_read(i2c_master_ctx, I2C_SLAVE_ADDR, read, I2C_MASTER_READ_TEST_SIZE, 1);
//         }
//         i2c_printf("master read test done");
//
//         i2c_printf("master read multiple test start");
//         {
//             uint8_t read[I2C_MASTER_READ_TEST_SIZE] = {0};
//             ret = rtos_i2c_master_read(i2c_master_ctx, I2C_SLAVE_ADDR, read, 1, 1);
//             if ((ret != I2C_ACK) || (read[0] != read_multiple_test_vector[0]))
//             {
//                 i2c_printf("master read multiple test failed on iteration 0");
//                 return -1;
//             }
//
//             ret = rtos_i2c_master_read(i2c_master_ctx, I2C_SLAVE_ADDR, read+1, I2C_MASTER_READ_TEST_SIZE-1, 1);
//             if (ret != I2C_ACK)
//             {
//                 i2c_printf("master read multiple test failed");
//                 return -1;
//             }
//             for (int i=1; i<I2C_MASTER_READ_TEST_SIZE; i++)
//             {
//                 if (read_multiple_test_vector[i] != read[i+1])
//                 {
//                     i2c_printf("master read multiple test failed on iteration %d", i);
//                 }
//             }
//         }
//         i2c_printf("master read multiple test done");
//     }
//     #endif
//
//     #if ON_TILE(1)
//     {
//         while(test0_slave_iters < I2C_MASTER_REG_WRITE_TEST_ITER)
//         {
//             vTaskDelay(pdMS_TO_TICKS(10));
//         }
//         i2c_printf("master reg write test done");
//
//         while(test1_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
//         {
//             vTaskDelay(pdMS_TO_TICKS(10));
//         }
//         i2c_printf("master reg read test done");
//
//         // while(test2_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
//         // {
//         //     vTaskDelay(pdMS_TO_TICKS(10));
//         // }
//         // i2c_printf("master write test done");
//         //
//         // while(test3_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
//         // {
//         //     vTaskDelay(pdMS_TO_TICKS(10));
//         // }
//         // i2c_printf("master write multiple test done");
//         //
//         // while(test4_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
//         // {
//         //     vTaskDelay(pdMS_TO_TICKS(10));
//         // }
//         // i2c_printf("master read test done");
//         //
//         // while(test5_slave_iters < I2C_MASTER_REG_READ_TEST_ITER)
//         // {
//         //     vTaskDelay(pdMS_TO_TICKS(10));
//         // }
//         // i2c_printf("master read multiple test done");
//     }
//     #endif
// #endif
//
//     return 0;
// }


static int run_i2c_tests(i2c_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            I2C_MAIN_TEST_ATTR i2c_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
#if ON_TILE(0)
            i2c_printf("MASTER missing main_test callback on test %d", test_ctx->cur_test);
#endif
#if ON_TILE(1)
            i2c_printf("SLAVE missing main_test callback on test %d", test_ctx->cur_test);
#endif
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_i2c_devices(i2c_test_ctx_t *test_ctx)
{
    i2c_printf("MASTER rpc configure");
    rtos_i2c_master_rpc_config(test_ctx->i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(0)
    i2c_printf("MASTER device start");
    rtos_i2c_master_start(test_ctx->i2c_master_ctx);
#endif

#if ON_TILE(1)
    i2c_printf("SLAVE start");
    rtos_i2c_slave_start(test_ctx->i2c_slave_ctx,
                         test_ctx,
                         i2c_slave_start,
                         i2c_slave_rx,
                         i2c_slave_tx_start,
                         i2c_slave_tx_done,
                         I2C_SLAVE_ISR_CORE,
                         configMAX_PRIORITIES-1);
#endif

    i2c_printf("Devices setup done");
}

static void register_i2c_tests(i2c_test_ctx_t *test_ctx)
{
    register_master_reg_write_test(test_ctx);
    register_master_reg_read_test(test_ctx);
    register_master_write_test(test_ctx);
    register_master_write_multiple_test(test_ctx);
    register_master_read_test(test_ctx);
    register_master_read_multiple_test(test_ctx);

    register_rpc_master_reg_write_test(test_ctx);
    register_rpc_master_reg_read_test(test_ctx);
    register_rpc_master_write_test(test_ctx);
    register_rpc_master_write_multiple_test(test_ctx);
    register_rpc_master_read_test(test_ctx);
    register_rpc_master_read_multiple_test(test_ctx);
}

static void i2c_init_tests(i2c_test_ctx_t *test_ctx, rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx)
{
    memset(test_ctx, 0, sizeof(i2c_test_ctx_t));
    test_ctx->i2c_master_ctx = i2c_master_ctx;
    test_ctx->i2c_slave_ctx = i2c_slave_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_i2c_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= I2C_MAX_TESTS);
}

void i2c_device_tests(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx, chanend_t c)
{
    i2c_test_ctx_t test_ctx;

    sync(c);
    i2c_printf("Init test context");
    i2c_init_tests(&test_ctx, i2c_master_ctx, i2c_slave_ctx);
    i2c_printf("Test context init");

    sync(c);
    i2c_printf("Start devices");
    start_i2c_devices(&test_ctx);
    i2c_printf("Devices started");

    sync(c);
    i2c_printf("Start tests");
    if (run_i2c_tests(&test_ctx, c) != 0)
    {
        i2c_printf("Test failed");
    } else {
        i2c_printf("Tests passed");
    }

    sync(c);   // Sync before return
}
