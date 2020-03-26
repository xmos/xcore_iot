// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */

/* App headers */
#include "app_conf.h"
#include "intertile_pipe_mgr_internal.h"

static IntertilePipe_t intertile_pipes[ appconfINTERTILE_MAX_PIPES ] = {NULL};

IntertilePipe_t get_intertile_pipe( int id )
{
    return intertile_pipes[ id ];
}

static void add_pipe( IntertilePipe_t pxPipe )
{
    static int cnt = 0;
    int start = cnt;

    do
    {
        if( intertile_pipes[ cnt ] == NULL )
        {
            intertile_pipes[ cnt ] = pxPipe;
            break;
        }
        cnt = ( cnt >= appconfINTERTILE_MAX_PIPES - 1) ? 0 : cnt+1;
    } while( start != cnt );
}

static void remove_pipe( IntertilePipe_t pxPipe )
{
    for( int i=0; i<appconfINTERTILE_MAX_PIPES; i++ )
    {
        if( intertile_pipes[ i ] == pxPipe )
        {
            intertile_pipes[ i ] = NULL;
            break;
        }
    }
}

/* Create intertile pipe, mapped to cb_id */
IntertilePipe_t intertile_pipe( intertile_cb_id_t cb_id )
{
    IntertilePipe_t pxIntertilePipe;

    pxIntertilePipe = pvPortMalloc( sizeof( prvIntertilePipe_t ) );

    if( pxIntertilePipe != NULL )
    {
        pxIntertilePipe->xReceivedBlockTime = portMAX_DELAY;
        pxIntertilePipe->xSendBlockTime = portMAX_DELAY;
        pxIntertilePipe->cb_id = cb_id;
        pxIntertilePipe->xRxQueue = xQueueCreate(1, sizeof( IntertileBufferDescriptor_t* ) );
        add_pipe( pxIntertilePipe );
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
