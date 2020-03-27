// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_PIPE_MGR_INTERNAL_H_
#define INTERTILE_PIPE_MGR_INTERNAL_H_

#include "intertile_driver.h"
#include "queue.h"
#include "soc.h"

#include "intertile_pipe_mgr.h"

#define TILE_PRINTF(fmt, ...) rtos_printf("tile[%d] "#fmt" \n", 1&get_local_tile_id(), ##__VA_ARGS__)

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
    int addr;
} prvIntertilePipe_t;

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

/* Intertile buffer descriptor type */
typedef struct IntertileBufferDescriptor
{
    ListItem_t xBufferListItem;
    soc_peripheral_t dev; /**< Intertile device used to transfer this buffer. */
    intertile_cb_footer_t *ftr;
    uint8_t *pucBuffer;
    size_t xLen;
    int addr;
} IntertileBufferDescriptor_t;

typedef struct IntertilePipeManagerArgs
{
    soc_peripheral_t dev;
    intertile_cb_id_t cb_id;
} IntertilePipeManagerArgs_t;

/* Function prototypes */
IntertilePipe_t get_intertile_pipe( intertile_cb_id_t id, int ndx );
void xIntertileDescriptorBuffersInit( void );
void vReleaseIntertileBufferAndDescriptor( IntertileBufferDescriptor_t * const pxBuffer );
IntertileBufferDescriptor_t *pxGetIntertileBufferWithDescriptor( size_t xRequestedSizeBytes, TickType_t xBlockTimeTicks );
IntertileBufferDescriptor_t *pxGetDescriptorFromBuffer( const void *pvBuffer );

#endif /* INTERTILE_PIPE_MGR_INTERNAL_H_ */
