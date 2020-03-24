// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_CTRL_H_
#define INTERTILE_CTRL_H_

void intertile_ctrl_create_t0( UBaseType_t uxPriority );
void intertile_ctrl_create_t1( UBaseType_t uxPriority );

#include "intertile_driver.h"
INTERTILE_ISR_CALLBACK_FUNCTION_PROTO( intertile_dev_test0, device, buf, len, status, xReturnBufferToDMA );
INTERTILE_ISR_CALLBACK_FUNCTION_PROTO( intertile_dev_test1, device, buf, len, status, xReturnBufferToDMA );

void t0_test(void *arg);
void t1_test(void *arg);


#include "queue.h"
#include "soc.h"

/* Intertile event types */
typedef enum
{
    eNoEvent = -1,
    eIntertileRxEvent,
    eIntertileTxEvent,
} eIntertileEvent_t;

/* Intertile event */
typedef struct IntertileEvent
{
    eIntertileEvent_t eEventType;
    void *pvData;
} IntertileEvent_t;

/* Intertile pipe event types */
typedef enum eIntertilePipeEvent {
    ePIPE_RECEIVE = 0x0001,
} eIntertilePipeEvent_t;

/* Intertile pipe internal usage */
typedef struct IntertilePipe
{
    QueueHandle_t xRxQueue;
    TickType_t xReceivedBlockTime;
    TickType_t xSendBlockTime;
    intertile_cb_id_t cb_id;
} prvIntertilePipe_t;

/* Intertile pipe */
typedef struct IntertilePipe *IntertilePipe_t;

/* Intertile buffer descriptor type */
typedef struct IntertileBufferDescriptor
{
    ListItem_t xBufferListItem;
    uint8_t *pucBuffer;
    size_t xLen;
    intertile_cb_header_t *hdr;
} IntertileBufferDescriptor_t;

/* Function prototypes */
IntertilePipe_t intertile_pipe( intertile_cb_id_t cb_id );
BaseType_t intertile_pipe_free( IntertilePipe_t pxPipe );

BaseType_t intertile_recv( IntertilePipe_t pxIntertilePipe, void* pvBuffer, size_t xBufferLen );
BaseType_t intertile_send( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

BaseType_t intertile_recv_copy( IntertilePipe_t pxIntertilePipe, void* pvBuffer, size_t xBufferLen );
BaseType_t intertile_send_copy( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

BaseType_t xIntertilePipeManagerReady( void );

BaseType_t xSendEventStructToIntertileTask( const IntertileEvent_t *pxEvent, TickType_t xTimeout );

void vReleaseIntertileBufferAndDescriptor( IntertileBufferDescriptor_t * const pxBuffer );
IntertileBufferDescriptor_t *pxGetIntertileBufferWithDescriptor( size_t xRequestedSizeBytes, TickType_t xBlockTimeTicks );
IntertileBufferDescriptor_t *pxGetDescriptorFromBuffer( const void *pvBuffer );

#endif /* INTERTILE_CTRL_H_ */
