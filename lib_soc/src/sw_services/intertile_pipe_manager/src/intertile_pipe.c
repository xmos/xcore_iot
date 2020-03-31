// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "intertile_driver.h"

/* App headers */
#include "intertile_pipe_mgr_internal.h"

static IntertilePipe_t intertile_pipes[ INTERTILE_CB_ID_COUNT ][ impconfINTERTILE_MAX_PIPES ] = { { NULL } };
static SemaphoreHandle_t xIntertilePipeSemaphore[ INTERTILE_CB_ID_COUNT ] = { NULL };

BaseType_t xIntertilePipeInit( intertile_cb_id_t cb_id )
{
    BaseType_t xRetVal = pdFAIL;

    xIntertilePipeSemaphore[ cb_id ] = xSemaphoreCreateMutex();
    configASSERT( xIntertilePipeSemaphore[ cb_id ] );

    if( xIntertilePipeSemaphore[ cb_id ] != NULL )
    {
        xRetVal = pdPASS;
    }

    return xRetVal;
}

IntertilePipe_t get_intertile_pipe( intertile_cb_id_t id, int ndx )
{
    return intertile_pipes[ id ][ ndx ];
}

static void add_pipe( IntertilePipe_t pxPipe )
{
    static int cnt = 0;
    intertile_cb_id_t id = pxPipe->cb_id;
    int start = cnt;

    do
    {
        if( intertile_pipes[ id ][ cnt ] == NULL )
        {
            intertile_pipes[ id ][ cnt ] = pxPipe;
            break;
        }
        cnt = ( cnt >= impconfINTERTILE_MAX_PIPES - 1) ? 0 : cnt+1;
    } while( start != cnt );
}

static void remove_pipe( IntertilePipe_t pxPipe )
{
    intertile_cb_id_t id = pxPipe->cb_id;
    for( int i=0; i<impconfINTERTILE_MAX_PIPES; i++ )
    {
        if( intertile_pipes[ id ][ i ] == pxPipe )
        {
            intertile_pipes[ id ][ i ] = NULL;
            break;
        }
    }
}

/* Create intertile pipe */
IntertilePipe_t intertile_pipe( intertile_cb_id_t cb_id, int addr )
{
    IntertilePipe_t pxIntertilePipe = NULL;

    if( cb_id < INTERTILE_CB_ID_COUNT )
    {
        if( xIntertilePipeSemaphore[ cb_id ] != NULL )
        {
            pxIntertilePipe = pvPortMalloc( sizeof( prvIntertilePipe_t ) );

            if( pxIntertilePipe != NULL )
            {
                pxIntertilePipe->xReceivedBlockTime = portMAX_DELAY;
                pxIntertilePipe->xSendBlockTime = portMAX_DELAY;
                pxIntertilePipe->cb_id = cb_id;
                pxIntertilePipe->addr = addr;
                pxIntertilePipe->xRxQueue = xQueueCreate(1, sizeof( IntertileBufferDescriptor_t* ) );

                if( pxIntertilePipe->xRxQueue != NULL )
                {
                    if( xSemaphoreTake( xIntertilePipeSemaphore[ cb_id ], portMAX_DELAY ) == pdTRUE )
                    {
                        add_pipe( pxIntertilePipe );
                        xSemaphoreGive( xIntertilePipeSemaphore[ cb_id ] );
                    }
                }
                else
                {
                    vPortFree( pxIntertilePipe );
                    pxIntertilePipe = NULL;
                }
            }
        }
    }

    return pxIntertilePipe;
}

BaseType_t intertile_pipe_free( IntertilePipe_t pxPipe )
{
    BaseType_t xRetVal = pdFAIL;

    if( pxPipe != NULL)
    {
        intertile_cb_id_t cb_id = pxPipe->cb_id;

        if( cb_id < INTERTILE_CB_ID_COUNT )
        {
            if( xSemaphoreTake( xIntertilePipeSemaphore[ cb_id ], portMAX_DELAY ) == pdTRUE )
            {
                remove_pipe( pxPipe );

                while( uxQueueMessagesWaiting( pxPipe->xRxQueue ) > 0)
                {
                    IntertileBufferDescriptor_t* buf;

                    if( ( xQueueReceive( pxPipe->xRxQueue, &buf, portMAX_DELAY ) == pdTRUE ) )
                    {
                        vReleaseIntertileBufferAndDescriptor( buf );
                    }
                }
                vQueueDelete( pxPipe->xRxQueue );
                pxPipe->xRxQueue = NULL;

                xSemaphoreGive( xIntertilePipeSemaphore[ cb_id ] );

                vPortFree( pxPipe );
                xRetVal = pdPASS;
            }
        }
    }

    return xRetVal;
}
