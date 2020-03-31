// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "list.h"

/* Library headers */

/* BSP/bitstream headers */

/* App headers */
#include "app_conf.h"
#include "intertile_pipe_mgr_internal.h"

static SemaphoreHandle_t xIntertileBufferSemaphore = NULL;

static List_t xFreeIntertileBuffersList;
static IntertileBufferDescriptor_t xIntertileBufferDescriptors[ impconfNUM_INTERTILE_BUFFER_DESCRIPTORS ];

BaseType_t xIntertileDescriptorBuffersInit( void )
{
    BaseType_t xRetVal = pdFAIL;

    xIntertileBufferSemaphore = xSemaphoreCreateCounting( impconfNUM_INTERTILE_BUFFER_DESCRIPTORS, impconfNUM_INTERTILE_BUFFER_DESCRIPTORS );
    configASSERT( xIntertileBufferSemaphore );

    if( xIntertileBufferSemaphore != NULL )
    {
        vListInitialise( &xFreeIntertileBuffersList );

        for( int i=0; i<impconfNUM_INTERTILE_BUFFER_DESCRIPTORS; i++ )
        {
            /* Initialise and set the owner of the buffer list items. */
            xIntertileBufferDescriptors[ i ].pucBuffer = NULL;
            xIntertileBufferDescriptors[ i ].ftr = NULL;
            vListInitialiseItem( &( xIntertileBufferDescriptors[ i ].xBufferListItem ) );
            listSET_LIST_ITEM_OWNER( &( xIntertileBufferDescriptors[ i ].xBufferListItem ), &xIntertileBufferDescriptors[ i ] );

            /* Currently, all buffers are available for use. */
            vListInsert( &xFreeIntertileBuffersList, &( xIntertileBufferDescriptors[ i ].xBufferListItem ) );
        }
        xRetVal = pdPASS;
    }

    return xRetVal;
}

void vReleaseIntertileBufferAndDescriptor( IntertileBufferDescriptor_t * const pxBuffer )
{
    BaseType_t xListItemAlreadyInFreeList;

    if( pxBuffer->pucBuffer != NULL )
    {
        pxBuffer->pucBuffer -= sizeof( IntertileBufferDescriptor_t* );

        if( pxBuffer->pucBuffer != NULL )
        {
            vPortFree( pxBuffer->pucBuffer );
            pxBuffer->pucBuffer = NULL;
        }
    }

    taskENTER_CRITICAL();
    {
        xListItemAlreadyInFreeList = listIS_CONTAINED_WITHIN( &xFreeIntertileBuffersList, &( pxBuffer->xBufferListItem ) );

        if( xListItemAlreadyInFreeList == pdFALSE )
        {
            vListInsertEnd( &xFreeIntertileBuffersList, &( pxBuffer->xBufferListItem ) );
        }
    }
    taskEXIT_CRITICAL();

    if( xListItemAlreadyInFreeList == pdFALSE )
    {
        if ( xSemaphoreGive( xIntertileBufferSemaphore ) == pdTRUE )
        {
            ;   //TILE_PRINTF("buffer released and semaphore given back");
        }
    }
    else
    {
        TILE_PRINTF("buffer released and semaphore not given back");
    }
}

IntertileBufferDescriptor_t *pxGetDescriptorFromBuffer( const void *pvBuffer )
{
uint8_t *pucBuffer;
IntertileBufferDescriptor_t *pxResult;

    if( pvBuffer == NULL )
    {
        pxResult = NULL;
    }
    else
    {
        pucBuffer = ( uint8_t * ) pvBuffer;
        pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
        pxResult = * ( ( IntertileBufferDescriptor_t ** ) pucBuffer );
    }

    return pxResult;
}

uint8_t *pucGetIntertileBuffer( size_t xRequestedSizeBytes )
{
    uint8_t *pucBuffer;

    pucBuffer = ( uint8_t * ) pvPortMalloc( xRequestedSizeBytes + ( sizeof( IntertileBufferDescriptor_t* ) ) );
    configASSERT( pucBuffer );

    if( pucBuffer != NULL )
    {
        pucBuffer += sizeof( IntertileBufferDescriptor_t* );
    }

    return pucBuffer;
}

void vReleaseIntertileBuffer( uint8_t *pucBuffer )
{
    if( pucBuffer != NULL )
    {
        pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
        vPortFree( pucBuffer );
    }
}

IntertileBufferDescriptor_t *pxGetIntertileBufferWithDescriptor( size_t xRequestedSizeBytes, TickType_t xBlockTimeTicks )
{
    IntertileBufferDescriptor_t *pxReturn = NULL;

    if( xIntertileBufferSemaphore != NULL )
    {
        if( xSemaphoreTake( xIntertileBufferSemaphore, xBlockTimeTicks ) == pdPASS )
        {
            taskENTER_CRITICAL();
            {
                pxReturn = ( IntertileBufferDescriptor_t * ) listGET_OWNER_OF_HEAD_ENTRY( &xFreeIntertileBuffersList );
                uxListRemove( &( pxReturn->xBufferListItem ) );
            }
            taskEXIT_CRITICAL();

            configASSERT( pxReturn->pucBuffer == NULL );

            if( xRequestedSizeBytes > 0 )
            {
                pxReturn->pucBuffer = ( uint8_t * ) pvPortMalloc( xRequestedSizeBytes + ( sizeof( IntertileBufferDescriptor_t* ) ) );

                if( pxReturn->pucBuffer != NULL )
                {
                    pxReturn->xLen = xRequestedSizeBytes;

                    *( ( IntertileBufferDescriptor_t ** ) ( pxReturn->pucBuffer ) ) = pxReturn;
                    pxReturn->pucBuffer += sizeof( IntertileBufferDescriptor_t* );
                }
                else
                {
                    vReleaseIntertileBufferAndDescriptor( pxReturn );
                    pxReturn = NULL;
                }
            }
        }
    }

    return pxReturn;
}
