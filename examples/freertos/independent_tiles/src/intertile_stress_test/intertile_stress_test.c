// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <string.h>
#include <xcore/hwtimer.h>

#include "intertile_stress_test.h"

static void intertile_test1(void *arg)
{
    char *inmsg;
    char outmsg[1024] = "intertile_test_1";
    size_t len;
    rtos_intertile_t *ctx = arg;
    size_t bytes_left_to_send = 128*1024*sizeof(outmsg);
    uint32_t t1, t2;

    t1 = get_reference_time();

#if ON_TILE(0)
    rtos_intertile_tx(
            ctx,
            5,
            outmsg,
            sizeof(outmsg));
    bytes_left_to_send -= sizeof(outmsg);
#endif

    while (bytes_left_to_send) {

        len = rtos_intertile_rx(
                ctx,
                5,
                (void **) &inmsg,
                portMAX_DELAY);

        xassert(strcmp(outmsg, inmsg) == 0);
        vPortFree(inmsg);

        rtos_intertile_tx(
                ctx,
                5,
                outmsg,
                sizeof(outmsg));

        bytes_left_to_send -= sizeof(outmsg);
        if (bytes_left_to_send % (16*1024*1024) == 0) {
            t2 = get_reference_time();
            rtos_printf("%d. %d ticks\n", bytes_left_to_send, t2-t1);
            t1 = t2;
        }
    }

#if ON_TILE(0)
    len = rtos_intertile_rx(
            ctx,
            5,
            (void **) &inmsg,
            portMAX_DELAY);

    //rtos_printf("Got msg: \"%s\" on tile %d, port 5\n", inmsg, THIS_XCORE_TILE);
    vPortFree(inmsg);
    //vTaskDelay(pdMS_TO_TICKS(1000));
#endif

    rtos_printf("Completed intertile_test on tile %d\n", THIS_XCORE_TILE);
    vTaskDelete(NULL);
}

static void intertile_test2(void *arg)
{
    char *inmsg;
    char outmsg[1024] = "intertile_test_2";
    size_t len;
    rtos_intertile_t *ctx = arg;
    size_t bytes_left_to_send = 128*1024*sizeof(outmsg);
    uint32_t t1, t2;

    t1 = get_reference_time();

#if ON_TILE(0)
    rtos_intertile_tx(
            ctx,
            2,
            outmsg,
            sizeof(outmsg));
    bytes_left_to_send -= sizeof(outmsg);
#endif

    while (bytes_left_to_send) {

        len = rtos_intertile_rx(
                ctx,
                2,
                (void **) &inmsg,
                portMAX_DELAY);

        xassert(strcmp(outmsg, inmsg) == 0);
        vPortFree(inmsg);

        rtos_intertile_tx(
                ctx,
                2,
                outmsg,
                sizeof(outmsg));

        bytes_left_to_send -= sizeof(outmsg);
        if (bytes_left_to_send % (16*1024*1024) == 0) {
            t2 = get_reference_time();
            rtos_printf("%d. %d ticks\n", bytes_left_to_send, t2-t1);
            t1 = t2;
        }
    }

#if ON_TILE(0)
    len = rtos_intertile_rx(
            ctx,
            2,
            (void **) &inmsg,
            portMAX_DELAY);
    vPortFree(inmsg);
#endif

    rtos_printf("Completed intertile_test2 on tile %d\n", THIS_XCORE_TILE);
    vTaskDelete(NULL);
}

void intertile_stress_test_start(rtos_intertile_t *intertile1_ctx, rtos_intertile_t *intertile2_ctx)
{
    xTaskCreate((TaskFunction_t) intertile_test1,
                "intertile_test1",
                RTOS_THREAD_STACK_SIZE(intertile_test1),
                intertile1_ctx,
                configMAX_PRIORITIES/2-1,
                NULL);

    xTaskCreate((TaskFunction_t) intertile_test2,
                "intertile_test2",
                RTOS_THREAD_STACK_SIZE(intertile_test2),
                intertile2_ctx,
                configMAX_PRIORITIES/2-1,
                NULL);
}
