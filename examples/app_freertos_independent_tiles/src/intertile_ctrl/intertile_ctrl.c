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
    soc_peripheral_t dev = args;
    size_t len;

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0 );

    while( xIntertilePipeManagerReady() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }
#ifndef TEST_ZERO_COPY
    uint8_t buf[INTERTILE_DEV_BUFSIZE];
    for( ;; )
    {
        len = intertile_recv_copy( pipe, &buf );

        TILE_PRINTF("<-- recv task %d bytes: %s", len, buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#else
    for( ;; )
    {
        uint8_t* buf = NULL;
        len = intertile_recv( pipe, &buf );
        TILE_PRINTF("<-- recv task %d bytes: %s", len, (char*)buf);

        vTaskDelay(pdMS_TO_TICKS(100));
        vReleaseIntertileBuffer( buf );
        vTaskDelay(pdMS_TO_TICKS(900));
    }
#endif
}

static void test_send( void* args)
{
    soc_peripheral_t dev = args;

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0 );

    uint8_t buf[] = "Hello";

    while( xIntertilePipeManagerReady() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }
#ifndef TEST_ZERO_COPY
    for( ;; )
    {
        TILE_PRINTF("--> send task %d bytes: %s", strlen((char *)buf) + 1 , (char*)buf);
        intertile_send_copy( pipe, &buf, strlen((char *)buf) + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#else
    for( ;; )
    {
        TILE_PRINTF("--> send task %d bytes: %s", strlen((char *)buf) + 1 , (char*)buf);
        uint8_t* sndbuf = pucGetIntertileBuffer( strlen((char *)buf) + 1 );
        memcpy(sndbuf, buf, strlen((char *)buf) + 1);
        intertile_send( pipe, sndbuf, strlen((char *)buf) + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#endif
}

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    IntertilePipeManagerInit(BITSTREAM_INTERTILE_DEVICE_A);

    xTaskCreate(test_recv, "test_recv", portTASK_STACK_DEPTH(test_recv), NULL, uxPriority, NULL);
}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    IntertilePipeManagerInit(BITSTREAM_INTERTILE_DEVICE_A);

    xTaskCreate(test_send, "test_send", portTASK_STACK_DEPTH(test_send), NULL, uxPriority, NULL);
}
