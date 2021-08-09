// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT QUEUE_TO_TCP
/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* App headers */
#include "app_conf.h"
#include "queue_to_tcp_stream/queue_to_tcp_stream.h"

static void queue_to_tcp_sender(void *arg)
{
	queue_to_tcp_handle_t handle = ( queue_to_tcp_handle_t ) arg;
    Socket_t xConnectedSocket = handle->socket;
    size_t data_length = handle->data_length;
    BaseType_t xSent = 0;

    int32_t *data = NULL;
    for (;;) {

        xQueueReceive( handle->queue, &data, portMAX_DELAY );

		xSent = FreeRTOS_send( xConnectedSocket,
							   data,
							   data_length,
							   0);

		if( xSent != data_length)
		{
			handle->connected = pdFALSE;

			FreeRTOS_shutdown( xConnectedSocket, FREERTOS_SHUT_RDWR );

			char dummy;
			while( FreeRTOS_recv( xConnectedSocket, &dummy, 1, 0 ) >= 0 )
			{
				vTaskDelay(pdMS_TO_TICKS( 100 ));
			}

			configASSERT( FreeRTOS_issocketconnected( xConnectedSocket ) == pdFALSE );
			FreeRTOS_closesocket( xConnectedSocket );

			debug_printf("Connection closed\n");
			debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
			debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

			vPortFree(data);

			vTaskDelete( NULL );
		}

		vPortFree(data);
    }
}

BaseType_t is_queue_to_tcp_connected( queue_to_tcp_handle_t handle )
{
	return handle->connected;
}

queue_to_tcp_handle_t queue_to_tcp_create(
		QueueHandle_t queue,
		uint16_t port,
		TickType_t rx_timeout,
		TickType_t tx_timeout,
		size_t data_length)
{
	queue_to_tcp_ctx_t *ptr;

	ptr = pvPortMalloc( sizeof( queue_to_tcp_ctx_t ) );

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

	return ( queue_to_tcp_handle_t )ptr;
}

static void queue2tcp( void *arg )
{
	queue_to_tcp_handle_t handle = ( queue_to_tcp_handle_t ) arg;

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
        xTaskCreate( queue_to_tcp_sender, "q2tcp_send", portTASK_STACK_DEPTH( queue_to_tcp_sender ), ( void * ) handle, uxTaskPriorityGet( NULL ), NULL );
    }
}

void queue_to_tcp_stream_create( queue_to_tcp_handle_t handle, UBaseType_t priority )
{
    xTaskCreate( queue2tcp, "q2tcp_listen", portTASK_STACK_DEPTH( queue2tcp ), ( void * ) handle, priority, NULL );
}
