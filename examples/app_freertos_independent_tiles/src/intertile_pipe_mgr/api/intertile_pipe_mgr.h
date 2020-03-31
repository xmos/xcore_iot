// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_PIPE_MGR_H_
#define INTERTILE_PIPE_MGR_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "intertile_pipe_mgr_conf_defaults.h"

/**
 * Initialize the intertile pipe manager
 *
 * Initializes a pipe manager for given device, and intertile
 * driver id.
 *
 * \param[in]     device_id      Intertile device id to use
 * \param[in]     cb_id          Intertile device driver callback to use
 *
 * \returns       pdPASS if manager task is sucessfully created
 *                pdFAIL otherwise
 */
BaseType_t IntertilePipeManagerInit( int device_id, intertile_cb_id_t cb_id );

/**
 * Check if the pipe manager is ready
 *
 * Checks if the pipe manager for a specified device is ready
 *
 * \param[in]     device_id      Intertile device id to check
 *
 * \returns       pdTRUE  if manager task is ready
 *                pdFALSE otherwise
 */
BaseType_t xIntertilePipeManagerReady( int device_id );

/**
 * Intertile pipe type
 */
typedef struct IntertilePipe *IntertilePipe_t;

/**
 * Create an intertile pipe
 *
 * The cb_id used must match the cb_id of the configured intertile
 * pipe manager.  The addr must be a unique identifer per cb_id
 * which is used by the pipe manager to direct recieved messages
 * to the pipe with matching addr.
 *
 * \param[in]     cb_id          Callback ID that this pipe will occur on
 *                               must be set to the same cb_id as the
 *                               pipe manager that will service messages
 * \param[in]     addr           The addr identifier
 *
 * \returns       A configured IntertilePipe_t on success
 *                NULL otherwise
 */
IntertilePipe_t intertile_pipe( intertile_cb_id_t cb_id, int addr );

/**
 * Frees an intertile pipe
 *
 * \param[in]     pxPipe         Pipe to free
 *
 * \returns       pdPASS  if pipe is sucessfully freed
 *                pdFAIL  otherwise
 */
BaseType_t intertile_pipe_free( IntertilePipe_t pxPipe );

/**
 * Recieve data from a pipe
 *
 * Note: User must free returned buffer with
 *       vReleaseIntertileBuffer when it is no
 *       longer needed
 *
 * \param[in]     pxIntertilePipe  Pipe to listen on
 * \param[in/out] pvBuffer         Buffer to read into
 *
 * \returns       number of bytes recieved
 */
BaseType_t intertile_recv( IntertilePipe_t pxIntertilePipe, void* pvBuffer );

/**
 * Send data to a pipe
 *
 * Note: User must provide a buffer which was created by
 *       pucGetIntertileBuffer.  The buffer will be internally
 *       freed when no longer needed, if sending is successful.
 *       If return value is not the length of the buffer sent
 *       then the buffer must be freed by the user.
 *
 * \param[in]     pxIntertilePipe  Pipe to send to
 * \param[in/out] pvBuffer         Buffer to send
 * \param[in]     xBufferLen       Length of buffer in bytes
 *
 * \returns       number of bytes sent
 */
BaseType_t intertile_send( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

/**
 * Recieve data from a pipe and copy into buffer
 *
 * Note: Data is copied so user can handle pvBuffer without restriction
 *
 * \param[in]     pxIntertilePipe  Pipe to listen on
 * \param[in/out] pvBuffer         Buffer to copy into
 *
 * \returns       number of bytes recieved
 */
BaseType_t intertile_recv_copy( IntertilePipe_t pxIntertilePipe, void* pvBuffer );

/**
 * Send copy of data to a pipe
 *
 * Note: Data is copied so user can handle pvBuffer without restriction
 *
 * \param[in]     pxIntertilePipe  Pipe to send to
 * \param[in/out] pvBuffer         Buffer to send
 * \param[in]     xBufferLen       Length of buffer in bytes
 *
 * \returns       number of bytes sent
 */
BaseType_t intertile_send_copy( IntertilePipe_t pxIntertilePipe, const void* pvBuffer, size_t xBufferLen );

/**
 * Get an intertile buffer
 *
 * \param[in]     xRequestedSizeBytes   Number of bytes requested
 *
 * \returns       An intertile buffer pointer
 *                NULL otherwise
 */
uint8_t *pucGetIntertileBuffer( size_t xRequestedSizeBytes );

/**
 * Release an intertile buffer
 *
 * \param[in]     pucBuffer   Intertile buffer pointer to free
 *
 * \returns       None
 */
void vReleaseIntertileBuffer( uint8_t *pucBuffer );

#endif /* INTERTILE_PIPE_MGR_H_ */
