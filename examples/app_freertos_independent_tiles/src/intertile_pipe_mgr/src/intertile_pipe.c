// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_pipe_mgr_internal.h"

static IntertilePipe_t intertile_pipes[ INTERTILE_CB_ID_COUNT ][ appconfINTERTILE_MAX_PIPES ] = {NULL};

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
        cnt = ( cnt >= appconfINTERTILE_MAX_PIPES - 1) ? 0 : cnt+1;
    } while( start != cnt );
}

static void remove_pipe( IntertilePipe_t pxPipe )
{
    intertile_cb_id_t id = pxPipe->cb_id;
    for( int i=0; i<appconfINTERTILE_MAX_PIPES; i++ )
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
    IntertilePipe_t pxIntertilePipe;

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
            add_pipe( pxIntertilePipe );
        }
        else
        {
            vPortFree( pxIntertilePipe );
            pxIntertilePipe = NULL;
        }
    }

    return pxIntertilePipe;
}

BaseType_t intertile_pipe_free( IntertilePipe_t pxPipe )
{
    BaseType_t xRetVal = pdFALSE;

    if( pxPipe != NULL)
    {
        remove_pipe( pxPipe );
        vQueueDelete( pxPipe->xRxQueue );
        if( pxPipe->xRxQueue == NULL )
        {
            vPortFree( pxPipe );
            if( pxPipe == NULL )
            {
                xRetVal = pdTRUE;
            }
        }
    }

    return xRetVal;
}
