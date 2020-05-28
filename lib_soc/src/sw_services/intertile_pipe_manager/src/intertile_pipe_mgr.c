// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* Standard library headers */
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Library headers */
#include "soc.h"
#include "rtos_support.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "intertile_pipe_mgr_internal.h"


static BaseType_t xPipeManagerReady[ BITSTREAM_INTERTILE_DEVICE_COUNT ] = { pdFALSE };
static QueueHandle_t xIntertileEventQueue = NULL;
static TaskHandle_t xIntertileTaskHandle = NULL;

static BaseType_t xSendEventStructToIntertileTask( const IntertileEvent_t *pxEvent, TickType_t xTimeout )
{
    BaseType_t xRetVal;
    /* The intertile task cannot block itself while waiting for itself to respond. */
    if( ( xTaskGetCurrentTaskHandle() == xIntertileTaskHandle ) == pdTRUE )
    {
        xRetVal = xQueueSendToBack( xIntertileEventQueue, pxEvent, 0 );
    }
    else
    {
        xRetVal = xQueueSendToBack( xIntertileEventQueue, pxEvent, xTimeout );
    }

    if( xRetVal == pdFAIL )
    {
        TILE_PRINTF("Failed to send event to intertile task",0);
    }

    return xRetVal;
}

portTIMER_CALLBACK_ATTRIBUTE
static void vDeferredIntertileTransmit( void *buf, uint32_t len )
{
    IntertileBufferDescriptor_t *pxBuffer;
    if( len > 0)
    {
        pxBuffer = pxGetDescriptorFromBuffer( buf );
        configASSERT( pxBuffer != NULL );
        vReleaseIntertileBufferAndDescriptor( pxBuffer );
    }
}

portTIMER_CALLBACK_ATTRIBUTE
static void vDeferredIntertileReceive( uint8_t *buf, int len )
{
    IntertileBufferDescriptor_t *pxNewBufDesc;
    IntertileBufferDescriptor_t *pxBufDesc;
    soc_peripheral_t dev;
    soc_dma_ring_buf_t *rx_ring_buf;

    pxBufDesc = pxGetDescriptorFromBuffer( buf );
    configASSERT( pxBufDesc != NULL );
    dev = pxBufDesc->dev;
    rx_ring_buf = soc_peripheral_rx_dma_ring_buf(dev);

    /* Allocate a new network buffer for the DMA engine
     * to replace the one it just gave us. */
    pxNewBufDesc = pxGetIntertileBufferWithDescriptor( INTERTILE_DEV_BUFSIZE, 0 );

    if( pxNewBufDesc != NULL )
    {
        IntertileEvent_t xRecvMessage;

        pxNewBufDesc->dev = dev;
        soc_dma_ring_rx_buf_set(rx_ring_buf, pxNewBufDesc->pucBuffer, INTERTILE_DEV_BUFSIZE);
        soc_peripheral_hub_dma_request(dev, SOC_DMA_RX_REQUEST);

        pxBufDesc->pucBuffer = ( uint8_t* )buf;
        pxBufDesc->xLen = len;

        xRecvMessage.eEventType = eIntertileRxEvent;
        xRecvMessage.pvData = ( void* )pxBufDesc;

        if( xSendEventStructToIntertileTask( &xRecvMessage, ( TickType_t ) 0 ) == pdFAIL )
        {
            TILE_PRINTF("failed to send rx data to intertile device",0);
            vReleaseIntertileBufferAndDescriptor( pxBufDesc );
        }
    }
    else
    {
        TILE_PRINTF("intertile data lost",0);

        /* Give the buffer back to the DMA engine */
        soc_dma_ring_rx_buf_set(rx_ring_buf, buf, INTERTILE_DEV_BUFSIZE);
        soc_peripheral_hub_dma_request(dev, SOC_DMA_RX_REQUEST);
    }
}

INTERTILE_ISR_CALLBACK_FUNCTION( intertile_dev_recv, device, buf, len, status, xReturnBufferToDMA)
{
    BaseType_t xYieldRequired = pdFALSE;

    if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM)
    {
        xTimerPendFunctionCallFromISR( (PendedFunction_t) vDeferredIntertileTransmit, buf, len, &xYieldRequired );
    }

    if (status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM)
    {
        BaseType_t xResult;
        xResult = xTimerPendFunctionCallFromISR(
                (PendedFunction_t) vDeferredIntertileReceive,
                buf,
                len,
                &xYieldRequired );

        if (xResult == pdPASS)
        {
            *xReturnBufferToDMA = pdFALSE;
        }
        else
        {
            *xReturnBufferToDMA = pdTRUE;
        }
    }

    return xYieldRequired;
}

BaseType_t intertile_recv( IntertilePipe_t pxIntertilePipe, void* pvBuffer )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* buf;

        if( xQueueReceive( xPipe->xRxQueue, &buf, xPipe->xReceivedBlockTime ) == pdTRUE )
        {
            *( (void**)pvBuffer ) = (void*)( &(buf->pucBuffer[0]) );
            xRetVal = (size_t)buf->xLen;

            /* We need to free the descriptor now so null pucBuffer
             * since the user is now in control of it */
            buf->pucBuffer = NULL;
            vReleaseIntertileBufferAndDescriptor( buf );
        }
        else
        {
            TILE_PRINTF("intertile recv timeout");
        }
    }
    else
    {
        TILE_PRINTF("Invalid pipe passed ro recv");
    }

    return xRetVal;
}

BaseType_t intertile_recv_copy( IntertilePipe_t pxIntertilePipe, void* pvBuffer )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* buf;

        if( xQueueReceive( xPipe->xRxQueue, &buf, xPipe->xReceivedBlockTime ) == pdTRUE )
        {
            memcpy( pvBuffer, (uint8_t*)buf->pucBuffer, (size_t)buf->xLen);
            xRetVal = (size_t)buf->xLen;

            vReleaseIntertileBufferAndDescriptor( buf );
        }
        else
        {
            TILE_PRINTF("intertile recv timeout",0);
        }
    }
    else
    {
        TILE_PRINTF("Invalid pipe passed ro recv",0);
    }

    return xRetVal;
}

BaseType_t intertile_send( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* pvData = pxGetIntertileBufferWithDescriptor( 0, 0 );

        if( pvData != NULL )
        {
            pvData->pucBuffer = ( uint8_t* )pvBuffer;

            // Populated the descriptor address
            pvData->pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
            *( ( IntertileBufferDescriptor_t ** ) ( pvData->pucBuffer ) ) = pvData;
            pvData->pucBuffer += sizeof( IntertileBufferDescriptor_t* );

            pvData->xLen = xBufferLen;
            pvData->ftr = ( intertile_cb_footer_t* )&xPipe->cb_id;
            pvData->addr = xPipe->addr;

            IntertileEvent_t xSendEvent;

            xSendEvent.eEventType = eIntertileTxEvent;
            xSendEvent.pvData = ( void* )pvData;
            if( xSendEventStructToIntertileTask( &xSendEvent, xPipe->xSendBlockTime ) == pdPASS )
            {
                xRetVal = xBufferLen;
            }
        }
        else
        {
            TILE_PRINTF("Failed to get buffer descriptor",0);
        }
    }
    else
    {
        TILE_PRINTF("Invalid pipe passed to send",0);
    }

    return xRetVal;
}

BaseType_t intertile_send_copy( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* pvData = pxGetIntertileBufferWithDescriptor( xBufferLen, 0 );

        if( pvData != NULL )
        {
            memcpy(pvData->pucBuffer, pvBuffer, xBufferLen);
            pvData->ftr = ( intertile_cb_footer_t* )&xPipe->cb_id;
            pvData->addr = xPipe->addr;

            IntertileEvent_t xSendEvent;

            xSendEvent.eEventType = eIntertileTxEvent;
            xSendEvent.pvData = ( void* )pvData;
            if( xSendEventStructToIntertileTask( &xSendEvent, xPipe->xSendBlockTime ) == pdPASS )
            {
                xRetVal = xBufferLen;
            }
        }
        else
        {
            TILE_PRINTF("Failed to get buffer descriptor",0);
        }
    }
    else
    {
        TILE_PRINTF("Invalid pipe passed to send",0);
    }

    return xRetVal;
}

static void prvHandleIntertilePacket( IntertileBufferDescriptor_t* const pxBuffer )
{
    /* cb_id is the first value of the footer */
    intertile_cb_id_t cb_id = pxBuffer->pucBuffer[ pxBuffer->xLen - sizeof( intertile_cb_footer_t ) ];

    uint8_t* ptr = pxBuffer->pucBuffer + pxBuffer->xLen - sizeof( intertile_cb_footer_t ) - sizeof( int );

    int addr;
    memcpy( &addr, ptr, sizeof(int) );

    IntertilePipe_t pxPipe = NULL;

    /*
     * TODO: should get the xIntertilePipeSemaphore here.
     * This will ensure that the pipe is not deleted
     * between getting the pipe and sending to its queue.
     */

    for( int i=0; i<impconfINTERTILE_MAX_PIPES; i++ )
    {
        pxPipe = get_intertile_pipe( cb_id, i );
        if( pxPipe != NULL )
        {
            if( ( pxPipe->addr ) == addr )
            {
                break;
            }
        }
    }

    /* We will update the length field to be only the size of the payload */
    pxBuffer->xLen -= sizeof( intertile_cb_footer_t );

    if( pxPipe != NULL )
    {
        configASSERT( pxPipe->xRxQueue != NULL );
        if( xQueueSend( pxPipe->xRxQueue, &pxBuffer, portMAX_DELAY ) == pdFAIL )
        {
            TILE_PRINTF("Failed to send to pipe[%d][%d] %d bytes lost", pxPipe->cb_id, pxPipe->addr, pxBuffer->xLen);
            vReleaseIntertileBufferAndDescriptor( pxBuffer );
        }
    }
    else
    {
        TILE_PRINTF("Failed to find pipe for id %d addr %d. %d bytes lost", cb_id, addr, pxBuffer->xLen);
        vReleaseIntertileBufferAndDescriptor( pxBuffer );
    }

    /* TODO: should release the xIntertilePipeSemaphore here */
}

static void prvIntertileSend( soc_peripheral_t device, IntertileBufferDescriptor_t * const pxBuffer )
{
    intertile_driver_send_bytes(
            device,
            (uint8_t*)pxBuffer->pucBuffer,
            (size_t)pxBuffer->xLen,
            (uint8_t*)&pxBuffer->addr,
            sizeof( unsigned ),
            (intertile_cb_footer_t*)pxBuffer->ftr );
}

BaseType_t xIntertilePipeManagerReady( int device_id )
{
    return xPipeManagerReady[ device_id ];
}

static void prvIntertilePipeManagerTask( void *pvArgs )
{
    IntertilePipeManagerArgs_t args = *( ( IntertilePipeManagerArgs_t* )pvArgs );
    soc_peripheral_t device = (args.dev);
    intertile_cb_id_t cb_id = (args.cb_id);

    xIntertileEventQueue = xQueueCreate( impconfINTERTILE_EVENT_QUEUE_LEN, sizeof( IntertileEvent_t ) );
    configASSERT( xIntertileEventQueue );

    xIntertilePipeInit( cb_id );
    xIntertileDescriptorBuffersInit();

    for( int i = 0; i < impconfNUM_RX_INTERTILE_BUFFER_DESCRIPTORS; i++ )
    {
        IntertileBufferDescriptor_t *bufdesc = pxGetIntertileBufferWithDescriptor(INTERTILE_DEV_BUFSIZE, 0);
        configASSERT(bufdesc != NULL);
        bufdesc->dev = device;
//        TILE_PRINTF("rx buffer desc @ %p with buf @ %p", bufdesc, bufdesc->pucBuffer);
        soc_dma_ring_rx_buf_set(soc_peripheral_rx_dma_ring_buf(device), bufdesc->pucBuffer, INTERTILE_DEV_BUFSIZE);
    }
    soc_peripheral_hub_dma_request(device, SOC_DMA_RX_REQUEST);

    intertile_cb_footer_t msg_ftr;
    intertile_driver_footer_init(&msg_ftr, cb_id);
    intertile_driver_register_callback( device, intertile_dev_recv, &msg_ftr);

    IntertileEvent_t xReceivedEvent;

    TILE_PRINTF("Intertile pipe manager ready");

    xPipeManagerReady[ soc_peripheral_get_id( device ) ] = pdTRUE;

    for( ;; )
    {
        if( xQueueReceive ( xIntertileEventQueue, ( void * ) &xReceivedEvent, portMAX_DELAY) == pdTRUE )
        {
            switch( xReceivedEvent.eEventType )
            {
            case eIntertileRxEvent:
                prvHandleIntertilePacket( ( IntertileBufferDescriptor_t * ) ( xReceivedEvent.pvData ) );
                break;
            case eIntertileTxEvent:
                prvIntertileSend( device, ( IntertileBufferDescriptor_t * ) ( xReceivedEvent.pvData ) );
                break;
            default:
                TILE_PRINTF("Invalid Intertile Event",0);
                break;
            }
        }
    }
}

BaseType_t IntertilePipeManagerInit( int device_id, intertile_cb_id_t cb_id, UBaseType_t uxPriority )
{
    BaseType_t xRetVal;

    soc_peripheral_t dev = intertile_driver_init( device_id,
                                                  impconfNUM_INTERTILE_BUFFER_DESCRIPTORS,
                                                  impconfNUM_INTERTILE_BUFFER_DESCRIPTORS,
                                                  0);

    static IntertilePipeManagerArgs_t args;
    args.dev = dev;
    args.cb_id = cb_id;

    xRetVal = xTaskCreate( prvIntertilePipeManagerTask,
                           "IntertilePipeMNGR",
                           portTASK_STACK_DEPTH( prvIntertilePipeManagerTask ),
                           &args,
                           uxPriority,
                           &xIntertileTaskHandle );

    return xRetVal;
}
