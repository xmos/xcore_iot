// Copyright (c) 2020, XMOS Ltd, All rights reserved

#define DEBUG_UNIT TCP_TO_QUEUE
#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"

/* App headers */
#include "tcp_stream_to_queue.h"


static void tcp_to_queue_receiver( void *arg )
{
	tcp_to_queue_handle_t handle = ( tcp_to_queue_handle_t ) arg;
    Socket_t xConnectedSocket = handle->socket;
    size_t data_length = handle->data_length;
    int8_t *data;
    BaseType_t bytes_rx = 0;

    for (;;) {
    	data = pvPortMalloc( sizeof(int8_t) * handle->data_length );
    	memset( data, 0x00, sizeof(int8_t) * handle->data_length );

    	if( data != NULL )
    	{
    		bytes_rx = FreeRTOS_recv( xConnectedSocket, data, handle->data_length, 0 );

    		if( bytes_rx > 0 )
    		{

    			if (xQueueSend( handle->queue, &data, pdMS_TO_TICKS(1)) == errQUEUE_FULL )
    			{
    				vPortFree(data);
    			}
    		}
    		else
    		{
				FreeRTOS_shutdown( xConnectedSocket, FREERTOS_SHUT_RDWR );


				char dummy;
				while( FreeRTOS_recv( xConnectedSocket, &dummy, 1, 0 ) >= 0 )
				{
					vTaskDelay(pdMS_TO_TICKS( 100 ));
				}

				FreeRTOS_closesocket( xConnectedSocket );

				debug_printf("Connection closed\n");
				debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
				debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

				vPortFree(data);

				vTaskDelete( NULL );
    		}
    	}
    }
}

BaseType_t is_tcp_to_queue_connected( tcp_to_queue_handle_t handle )
{
	return handle->connected;
}

tcp_to_queue_handle_t tcp_to_queue_create(
		QueueHandle_t queue,
		uint16_t port,
		TickType_t rx_timeout,
		TickType_t tx_timeout,
		size_t data_length)
{
	tcp_to_queue_ctx_t *ptr;

	ptr = pvPortMalloc( sizeof( tcp_to_queue_ctx_t ) );

	if( ptr != NULL )
	{
		ptr->queue = queue;
		ptr->rx_timeout = rx_timeout;
		ptr->tx_timeout = tx_timeout;
		ptr->connected = pdFALSE;
		ptr->data_length = data_length;
		ptr->socket = NULL;
		ptr->port = port;
	}

	return ( tcp_to_queue_handle_t )ptr;
}

static void tcp2queue( void *arg )
{
	tcp_to_queue_handle_t handle = ( tcp_to_queue_handle_t ) arg;

    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    const TickType_t xReceiveTimeOut = handle->rx_timeout;
    const TickType_t xSendTimeOut = handle->tx_timeout;
    const BaseType_t xBacklog = 1;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS( 100 ));
    }

    /* Attempt to open the socket. */
    xListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET,
										FREERTOS_SOCK_STREAM,
										FREERTOS_IPPROTO_TCP );

    /* Check the socket was created. */
    configASSERT( xListeningSocket != FREERTOS_INVALID_SOCKET );

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt( xListeningSocket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeOut,
                         sizeof( xReceiveTimeOut ) );

    /* Set the listening port */
    xBindAddress.sin_port = FreeRTOS_htons( handle->port );

    /* Bind the socket to the port that the client RTOS task will send to. */
    FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

    /* Set the socket into a listening state so it can accept connections.
    The maximum number of simultaneous connections is limited to 20. */
    FreeRTOS_listen( xListeningSocket, xBacklog );

    for( ;; )
    {
        /* Wait for incoming connections. */
        xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );

        configASSERT( xConnectedSocket != FREERTOS_INVALID_SOCKET );

        FreeRTOS_setsockopt( xConnectedSocket,
                             0,
                             FREERTOS_SO_SNDTIMEO,
                             &xSendTimeOut,
                             sizeof( xSendTimeOut ) );

        handle->socket = xConnectedSocket;
        handle->connected = pdTRUE;
        xTaskCreate( tcp_to_queue_receiver, "tcp2q_rx", portTASK_STACK_DEPTH( tcp_to_queue_receiver ), ( void * ) handle, uxTaskPriorityGet( NULL ), NULL );
    }
}

void tcp_stream_to_queue_create( tcp_to_queue_handle_t handle, UBaseType_t priority )
{
    xTaskCreate( tcp2queue, "tcp2queue_listen", portTASK_STACK_DEPTH( tcp2queue ), ( void * ) handle, priority, NULL );
}
