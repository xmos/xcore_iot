// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_PIPE_MGR_H_
#define INTERTILE_PIPE_MGR_H_

BaseType_t IntertilePipeManagerInit( int device_id, intertile_cb_id_t cb_id );
BaseType_t xIntertilePipeManagerReady( void );

typedef struct IntertilePipe *IntertilePipe_t;

IntertilePipe_t intertile_pipe( intertile_cb_id_t cb_id, int addr );
BaseType_t intertile_pipe_free( IntertilePipe_t pxPipe );

BaseType_t intertile_recv( IntertilePipe_t pxIntertilePipe, void* pvBuffer );
BaseType_t intertile_send( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

BaseType_t intertile_recv_copy( IntertilePipe_t pxIntertilePipe, void* pvBuffer );
BaseType_t intertile_send_copy( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

uint8_t *pucGetIntertileBuffer( size_t xRequestedSizeBytes );
void vReleaseIntertileBuffer( uint8_t *pucBuffer );

#endif /* INTERTILE_PIPE_MGR_H_ */
