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
#include "intertile_ctrl.h"

static IntertilePipe_t intertile_pipes[ appconfINTERTILE_MAX_PIPES ] = {NULL};

static QueueHandle_t xIntertileEventQueue = NULL;
static TaskHandle_t xIntertileTaskHandle = NULL;

static BaseType_t xPipeManagerReady = pdFALSE;


static List_t xFreeIntertileBuffersList;
static IntertileBufferDescriptor_t xIntertileBufferDescriptors[ appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS ];
static SemaphoreHandle_t xIntertileBufferSemaphore = NULL;

#define TILE_PRINTF(fmt, ...) rtos_printf("tile[%d] "#fmt" \n", 1&get_local_tile_id(), ##__VA_ARGS__)


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
    TILE_PRINTF("rx debug descriptor is %p", pxBufDesc);
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

        pxBufDesc->hdr = (intertile_cb_header_t *) buf;
        pxBufDesc->pucBuffer = ( uint8_t* )buf + sizeof( intertile_cb_header_t );
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
    }
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
        pxIntertilePipe->xRxQueue = xQueueCreate(1, INTERTILE_DEV_BUFSIZE);
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

BaseType_t intertile_recv( IntertilePipe_t pxIntertilePipe, void* pvBuffer, size_t xBufferLen )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* buf = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );

        if( xQueueReceive( xPipe->xRxQueue, buf, xPipe->xReceivedBlockTime ) == pdTRUE )
        {
            uint8_t* abuf = (uint8_t*)buf->pucBuffer;
            xRetVal = (size_t)buf->xLen;
            intertile_cb_header_t* hdr = (intertile_cb_header_t*)buf->hdr;
//            rtos_printf("received buffer len %d %s\n", xRetVal, abuf);
        }
        else
        {
            TILE_PRINTF("intertile recv timeout",0);
        }

        vPortFree(buf);
    }
    else
    {
        TILE_PRINTF("Invalid pipe passed ro recv",0);
    }

    return xRetVal;
}

BaseType_t intertile_recv_copy( IntertilePipe_t pxIntertilePipe, void* pvBuffer, size_t xBufferLen )
{
    prvIntertilePipe_t *xPipe = ( prvIntertilePipe_t* ) pxIntertilePipe;
    BaseType_t xRetVal = 0;

    // Verify a configured intertile pipe was passed in
    if( xPipe->xRxQueue != NULL )
    {
        IntertileBufferDescriptor_t* buf = pvPortMalloc( sizeof(uint8_t) * INTERTILE_DEV_BUFSIZE );

        if( xQueueReceive( xPipe->xRxQueue, buf, xPipe->xReceivedBlockTime ) == pdTRUE )
        {
            memcpy( pvBuffer, (uint8_t*)buf->pucBuffer, (size_t)buf->xLen);
            xRetVal = (size_t)buf->xLen;
            intertile_cb_header_t* hdr = (intertile_cb_header_t*)buf->hdr;
//            rtos_printf("received buffer len %d %s\n", xRetVal, pvBuffer);
        }
        else
        {
            TILE_PRINTF("intertile recv timeout",0);
        }

        vPortFree(buf);
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
            // Populated the descriptor address
            pvData->pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
            *( ( IntertileBufferDescriptor_t ** ) ( pvData->pucBuffer ) ) = pvData;
            pvData->pucBuffer += sizeof( IntertileBufferDescriptor_t* );

            pvData->pucBuffer = pvBuffer;
            pvData->xLen = xBufferLen;
            pvData->hdr = &xPipe->cb_id;

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
            pvData->hdr = &xPipe->cb_id;

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

BaseType_t xIntertilePipeManagerReady( void )
{
    return xPipeManagerReady;
}

static void prvHandleIntertilePacket( IntertileBufferDescriptor_t* const pxBuffer )
{
    intertile_cb_header_t* hdr = (intertile_cb_header_t*)pxBuffer->hdr;
    intertile_cb_id_t cb_id = (intertile_cb_id_t)hdr->cb_id;
    IntertilePipe_t pxPipe = NULL;

    for( int i=0; i<appconfINTERTILE_MAX_PIPES; i++ )
    {
        pxPipe = intertile_pipes[i];
        if( ( pxPipe->cb_id ) == cb_id )
        {
            break;
        }
    }

    /* We will update the length field to be only the size of the payload */
    pxBuffer->xLen -= sizeof( intertile_cb_header_t );

    xQueueSend( pxPipe->xRxQueue, pxBuffer, portMAX_DELAY );

    /* The intertile device sends a buffer with header followed by data.
     * vDeferredIntertileRecieve places the start of data pointer
     * in pxBuffer->pucBuffer however, the start of the allocated buffer
     * is at pxBuffer->pucBuffer - sizeof( intertile_cb_header_t ) */
    pxBuffer->pucBuffer -= sizeof( intertile_cb_header_t );

    vReleaseIntertileBufferAndDescriptor( pxBuffer );
}

static void prvIntertileSend( soc_peripheral_t device, IntertileBufferDescriptor_t * const pxBuffer )
{
    intertile_driver_send_bytes(
            device,
            (uint8_t*)pxBuffer->pucBuffer,
            (size_t)pxBuffer->xLen,
            (intertile_cb_header_t*)pxBuffer->hdr );
}


void vReleaseIntertileBufferAndDescriptor( IntertileBufferDescriptor_t * const pxBuffer )
{
    BaseType_t xListItemAlreadyInFreeList;

    pxBuffer->pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
    if( pxBuffer->pucBuffer != NULL )
    {
        TILE_PRINTF("trying to free buffer at %p", pxBuffer->pucBuffer);
        vPortFree( pxBuffer->pucBuffer );
        pxBuffer->pucBuffer = NULL;
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
            TILE_PRINTF("buffer released and semaphore given back",0);
        }
    }
    else
    {
        TILE_PRINTF("buffer released and semaphore not given back",0);
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

static void prvIntertilePipeManagerTask( void *pvArgs )
{
    soc_peripheral_t device = pvArgs;
    TILE_PRINTF("prvIntertilePipeManagerTask",0);

    xIntertileEventQueue = xQueueCreate( app_confINTERTILE_EVENT_QUEUE_LEN, sizeof( IntertileEvent_t ) );
    configASSERT( xIntertileEventQueue );

    xIntertileBufferSemaphore = xSemaphoreCreateCounting( appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS, appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS );
    configASSERT( xIntertileBufferSemaphore );

    if( xIntertileBufferSemaphore != NULL )
    {
        vListInitialise( &xFreeIntertileBuffersList );

        for( int i=0; i<appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS; i++ )
        {
            /* Initialise and set the owner of the buffer list items. */
            xIntertileBufferDescriptors[ i ].pucBuffer = NULL;
            xIntertileBufferDescriptors[ i ].hdr = NULL;
            vListInitialiseItem( &( xIntertileBufferDescriptors[ i ].xBufferListItem ) );
            listSET_LIST_ITEM_OWNER( &( xIntertileBufferDescriptors[ i ].xBufferListItem ), &xIntertileBufferDescriptors[ i ] );

            /* Currently, all buffers are available for use. */
            vListInsert( &xFreeIntertileBuffersList, &( xIntertileBufferDescriptors[ i ].xBufferListItem ) );
        }
    }

    for( int i = 0; i < appconfigNUM_INTERTILE_RX_DMA_BUF; i++ )
    {
        IntertileBufferDescriptor_t *bufdesc = pxGetIntertileBufferWithDescriptor(INTERTILE_DEV_BUFSIZE, 0);
        configASSERT(bufdesc != NULL);
        TILE_PRINTF("rx buffer desc @ %p with buf @ %p", bufdesc, bufdesc->pucBuffer);
        bufdesc->dev = device;
        soc_dma_ring_rx_buf_set(soc_peripheral_rx_dma_ring_buf(device), bufdesc->pucBuffer, INTERTILE_DEV_BUFSIZE);
    }

    IntertileEvent_t xReceivedEvent;
    IntertilePipe_t *pxIntertilePipe;
    TILE_PRINTF("Pipe manager ready",0);

    xPipeManagerReady = pdTRUE;

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

BaseType_t xSendEventStructToIntertileTask( const IntertileEvent_t *pxEvent, TickType_t xTimeout )
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

        if (xResult != pdPASS)
        {
            *xReturnBufferToDMA = pdTRUE;
        }
    }

    return xYieldRequired;
}


#if THIS_XCORE_TILE == 0

int add(int a, int b)
{
    int ret;
    TILE_PRINTF("Add",0);
    // rpc call

    return ret;
}
#endif

#if THIS_XCORE_TILE == 1
int add(int a, int b)
{
    TILE_PRINTF("Add",0);

    return a+b;
}
#endif

#define TEST(a,b) {ret = add(a, b);rtos_printf("%d+%d=%d\n",a, b, ret);}

void add_task(void *arg)
{
    int ret, a, b;
    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(1,1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(-5,2);
        vTaskDelay(pdMS_TO_TICKS(1000));
        TEST(0,31);
    }
}

static void test_recv( void* args)
{
    soc_peripheral_t dev = args;
    size_t len;

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0 );

    intertile_cb_header_t msg_hdr;
    intertile_driver_header_init(&msg_hdr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_recv, &msg_hdr);

    while( xIntertilePipeManagerReady() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }

    uint8_t buf[INTERTILE_DEV_BUFSIZE];
    for( ;; )
    {
        len = intertile_recv_copy( pipe, &buf, INTERTILE_DEV_BUFSIZE);
        for(int i=0; i<len; i++)
        {
            TILE_PRINTF("recv task revc[%d]: %c", i, buf[i]);
        }

        TILE_PRINTF("recv task revc %d bytes: %s", len, buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS,
            appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS,
            NULL,
            0);

    xTaskCreate(prvIntertilePipeManagerTask, "IntertilePipeMNGR", portTASK_STACK_DEPTH(prvIntertilePipeManagerTask), dev, uxPriority, &xIntertileTaskHandle);

    xTaskCreate(test_recv, "test_recv", portTASK_STACK_DEPTH(test_recv), dev, uxPriority-1, NULL);

//    xTaskCreate(add_task, "add_task", portTASK_STACK_DEPTH(add_task), dev, uxPriority, NULL);
//    xTaskCreate(t0_test, "tile0_intertile", portTASK_STACK_DEPTH(t0_test), dev, uxPriority, NULL);

}

static void test_send( void* args)
{
    soc_peripheral_t dev = args;

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0 );

    intertile_cb_header_t msg_hdr;
    intertile_driver_header_init(&msg_hdr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_recv, &msg_hdr);

    uint8_t buf[] = "Hello";

    while( xIntertilePipeManagerReady() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // try again in 100ms
    }

    for( ;; )
    {
        intertile_send_copy( pipe, &buf, strlen((char *)buf) + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS,
            appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS,
            NULL,
            0);

    xTaskCreate(prvIntertilePipeManagerTask, "IntertilePipeMNGR", portTASK_STACK_DEPTH(prvIntertilePipeManagerTask), dev, uxPriority, &xIntertileTaskHandle);
    xTaskCreate(test_send, "test_send", portTASK_STACK_DEPTH(test_send), dev, uxPriority-1, NULL);

//    xTaskCreate(t1_test, "tile1_intertile", portTASK_STACK_DEPTH(t1_test), dev, uxPriority, NULL);
}
