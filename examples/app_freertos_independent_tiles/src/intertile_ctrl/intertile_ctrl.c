// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_pipe_mgr.h"


#define TILE_PRINTF(fmt, ...) rtos_printf("tile[%d] "#fmt" \n", 1&get_local_tile_id(), ##__VA_ARGS__)

#define TEST_ZERO_COPY  1

static void test_recv( void* args)
{
    int addr = *((int*)args);
    size_t len;

    while( xIntertilePipeManagerReady( BITSTREAM_INTERTILE_DEVICE_A ) == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0, addr );

#ifndef TEST_ZERO_COPY
    uint8_t buf[INTERTILE_DEV_BUFSIZE];
    for( ;; )
    {
        len = intertile_recv_copy( pipe, &buf );

        TILE_PRINTF("<-- recv at addr %d %d bytes: %s", addr, len, (char*)buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#else
    for( ;; )
    {
        uint8_t* buf = NULL;
        len = intertile_recv( pipe, &buf );
        TILE_PRINTF("<-- recv at addr %d %d bytes: %s", addr, len, (char*)buf);

        vTaskDelay(pdMS_TO_TICKS(100));
        vReleaseIntertileBuffer( buf );
        vTaskDelay(pdMS_TO_TICKS(900));
    }
#endif
}

static void test_send( void* args)
{
    int addr = *((int*)args);

    uint8_t buf[] = "Hello";

    while( xIntertilePipeManagerReady( BITSTREAM_INTERTILE_DEVICE_A ) == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0, addr );

    vTaskDelay(pdMS_TO_TICKS(100)); // delay so rx side has time to register pipes

#ifndef TEST_ZERO_COPY
    for( ;; )
    {
        TILE_PRINTF("--> send to addr %d %d bytes: %s", addr, strlen((char *)buf) + 1 , (char*)buf);
        intertile_send_copy( pipe, &buf, strlen((char *)buf) + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#else
    for( ;; )
    {
        TILE_PRINTF("--> send to addr %d %d bytes: %s", addr, strlen((char *)buf) + 1 , (char*)buf);
        uint8_t* sndbuf = pucGetIntertileBuffer( strlen((char *)buf) + 1 );
        memcpy(sndbuf, buf, strlen((char *)buf) + 1);
        intertile_send( pipe, sndbuf, strlen((char *)buf) + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#endif
}

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    IntertilePipeManagerInit( BITSTREAM_INTERTILE_DEVICE_A, INTERTILE_CB_ID_0, appconfINTERTILE_PIPE_MGR_TASK_PRIORITY );

    static int addr0 = 0;
    xTaskCreate(test_recv, "test_recv0", portTASK_STACK_DEPTH(test_recv), &addr0, uxPriority, NULL);

    static int addr1 = 1;
    xTaskCreate(test_recv, "test_recv1", portTASK_STACK_DEPTH(test_recv), &addr1, uxPriority, NULL);
}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    IntertilePipeManagerInit( BITSTREAM_INTERTILE_DEVICE_A, INTERTILE_CB_ID_0, appconfINTERTILE_PIPE_MGR_TASK_PRIORITY );

    static int addr0 = 0;
    xTaskCreate(test_send, "test_send0", portTASK_STACK_DEPTH(test_send), &addr0, uxPriority, NULL);

    static int addr1 = 1;
    xTaskCreate(test_send, "test_send1", portTASK_STACK_DEPTH(test_send), &addr1, uxPriority, NULL);
}
