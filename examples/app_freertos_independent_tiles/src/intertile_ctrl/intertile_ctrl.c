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


#define TEST_ZERO_COPY  1





static IntertilePipe_t intertile_pipes[ appconfINTERTILE_MAX_PIPES ] = {NULL};

static QueueHandle_t xIntertileEventQueue = NULL;
static TaskHandle_t xIntertileTaskHandle = NULL;

static BaseType_t xPipeManagerReady = pdFALSE;


static List_t xFreeIntertileBuffersList;
static IntertileBufferDescriptor_t xIntertileBufferDescriptors[ appconfigNUM_INTERTILE_BUFFER_DESCRIPTORS ];
static SemaphoreHandle_t xIntertileBufferSemaphore = NULL;


static soc_peripheral_t xDevice;


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
    IntertileBufferDescriptor_t *pxBuffer;
    soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(xDevice);

    /* Allocate a new network buffer for the DMA engine
     * to replace the one it just gave us. */
    pxBuffer = pxGetIntertileBufferWithDescriptor( INTERTILE_DEV_BUFSIZE, 0 );

    if( pxBuffer != NULL )
    {
        soc_dma_ring_rx_buf_set(rx_ring_buf, pxBuffer->pucBuffer, INTERTILE_DEV_BUFSIZE);
        soc_peripheral_hub_dma_request(xDevice, SOC_DMA_RX_REQUEST);

        IntertileEvent_t xRecvMessage;

        IntertileBufferDescriptor_t* pvData = pxGetDescriptorFromBuffer( buf );

        configASSERT( pvData != NULL );

        pvData->pucBuffer = ( uint8_t* )buf;
        pvData->xLen = len;

        xRecvMessage.eEventType = eIntertileRxEvent;
        xRecvMessage.pvData = ( void* )pvData;

        if( xSendEventStructToIntertileTask( &xRecvMessage, ( TickType_t ) 0 ) == pdFAIL )
        {
            TILE_PRINTF("failed to send rx data to intertile device",0);
            vReleaseIntertileBufferAndDescriptor( pvData );
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
            pvData->pucBuffer = pvBuffer;

            // Populated the descriptor address
            pvData->pucBuffer -= sizeof( IntertileBufferDescriptor_t* );
            *( ( IntertileBufferDescriptor_t ** ) ( pvData->pucBuffer ) ) = pvData;
            pvData->pucBuffer += sizeof( IntertileBufferDescriptor_t* );

            pvData->xLen = xBufferLen;
            pvData->ftr = &xPipe->cb_id;

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
            pvData->ftr = &xPipe->cb_id;

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
    intertile_cb_id_t cb_id;

    /* cb_id is the first value of the footer */
    cb_id = pxBuffer->pucBuffer[ pxBuffer->xLen - sizeof( intertile_cb_footer_t ) ];

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
    pxBuffer->xLen -= sizeof( intertile_cb_footer_t );

    if( xQueueSend( pxPipe->xRxQueue, &pxBuffer, portMAX_DELAY ) == pdFAIL )
    {
        TILE_PRINTF("Failed to send to pipe[%d] %d bytes lost", pxPipe->cb_id, pxBuffer->xLen);
        vReleaseIntertileBufferAndDescriptor( pxBuffer );
    }
}

static void prvIntertileSend( soc_peripheral_t device, IntertileBufferDescriptor_t * const pxBuffer )
{
    intertile_driver_send_bytes(
            device,
            (uint8_t*)pxBuffer->pucBuffer,
            (size_t)pxBuffer->xLen,
            (intertile_cb_footer_t*)pxBuffer->ftr );
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
            ;
//            TILE_PRINTF("buffer released and semaphore given back");
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

static void prvIntertilePipeManagerTask( void *pvArgs )
{
    soc_peripheral_t device = pvArgs;

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
            xIntertileBufferDescriptors[ i ].ftr = NULL;
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
//        TILE_PRINTF("rx buffer desc @ %p with buf @ %p", bufdesc, bufdesc->pucBuffer);
        soc_dma_ring_rx_buf_set(soc_peripheral_rx_dma_ring_buf(device), bufdesc->pucBuffer, INTERTILE_DEV_BUFSIZE);
    }

    IntertileEvent_t xReceivedEvent;
    IntertilePipe_t *pxIntertilePipe;
    TILE_PRINTF("Intertile pipe manager ready");

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
    /* The intertile task cannot block itself while waiting for it        }
     * self to respond. */
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
    soc_peripheral_t dev = device;
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

    intertile_cb_footer_t msg_ftr;
    intertile_driver_footer_init(&msg_ftr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_recv, &msg_ftr);

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

void intertile_ctrl_create_t0( UBaseType_t uxPriority )
{
    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            NULL,
            0);

    xDevice = dev;

    xTaskCreate(prvIntertilePipeManagerTask, "IntertilePipeMNGR", portTASK_STACK_DEPTH(prvIntertilePipeManagerTask), dev, uxPriority, &xIntertileTaskHandle);

    xTaskCreate(test_recv, "test_recv", portTASK_STACK_DEPTH(test_recv), dev, uxPriority-1, NULL);

//    xTaskCreate(add_task, "add_task", portTASK_STACK_DEPTH(add_task), dev, uxPriority, NULL);
//    xTaskCreate(t0_test, "tile0_intertile", portTASK_STACK_DEPTH(t0_test), dev, uxPriority, NULL);
}

static void test_send( void* args)
{
    soc_peripheral_t dev = args;

    IntertilePipe_t pipe = intertile_pipe( INTERTILE_CB_ID_0 );

    intertile_cb_footer_t msg_ftr;
    intertile_driver_footer_init(&msg_ftr, INTERTILE_CB_ID_0);
    intertile_driver_register_callback( dev, intertile_dev_recv, &msg_ftr);

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

void intertile_ctrl_create_t1( UBaseType_t uxPriority )
{
    soc_peripheral_t dev = intertile_driver_init(
            BITSTREAM_INTERTILE_DEVICE_A,
            2,
            2,
            NULL,
            0);

    xDevice = dev;

    xTaskCreate(prvIntertilePipeManagerTask, "IntertilePipeMNGR", portTASK_STACK_DEPTH(prvIntertilePipeManagerTask), dev, uxPriority, &xIntertileTaskHandle);
    xTaskCreate(test_send, "test_send", portTASK_STACK_DEPTH(test_send), dev, uxPriority-1, NULL);

//    xTaskCreate(t1_test, "tile1_intertile", portTASK_STACK_DEPTH(t1_test), dev, uxPriority, NULL);
}
