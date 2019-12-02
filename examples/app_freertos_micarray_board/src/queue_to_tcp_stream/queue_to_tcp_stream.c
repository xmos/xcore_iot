// Copyright (c) 2019, XMOS Ltd, All rights reserved

#define DEBUG_UNIT QUEUE_TO_TCP
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
#include "ethernet_driver.h"

/* App headers */
#include "queue_to_tcp_stream.h"

static QueueHandle_t queue_to_tcp;

static BaseType_t xConnected = 0;

BaseType_t is_queue_to_tcp_connected( void )
{
    return xConnected;
}

static void queue_to_tcp_sender(void *arg)
{
    Socket_t xConnectedSocket = ( Socket_t ) arg;
    const TickType_t xSendTimeOut = pdMS_TO_TICKS( 5000 );
    BaseType_t xSent;

    xConnected = pdTRUE;

    FreeRTOS_setsockopt( xConnectedSocket,
                         0,
                         FREERTOS_SO_SNDTIMEO,
                         &xSendTimeOut,
                         sizeof( xSendTimeOut ) );

    for (;;) {
        int32_t *audio_data;

        xQueueReceive(queue_to_tcp, &audio_data, portMAX_DELAY);

        xSent = FreeRTOS_send( xConnectedSocket,
                               audio_data,
                               sizeof(int32_t) * appconfMIC_FRAME_LENGTH,
                               0);

        if( xSent != ( sizeof(int32_t) * appconfMIC_FRAME_LENGTH ) )
        {
            debug_printf("Send to TCP lost\n");

            if( FreeRTOS_issocketconnected( xConnectedSocket ) == pdFALSE )
            {
                debug_printf("Remote disconnected, try to reconnect\n");
            }
            else
            {
                debug_printf("Connection broken, shutting down connection\n");

                debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

                FreeRTOS_shutdown( xConnectedSocket, FREERTOS_SHUT_RDWR );
                char dummy;
                while( FreeRTOS_recv( xConnectedSocket, &dummy, 1, 0 ) >= 0 )
                {
                    vTaskDelay(pdMS_TO_TICKS( 100 ));
                }

                configASSERT( FreeRTOS_issocketconnected( xConnectedSocket ) == pdFALSE );
                FreeRTOS_closesocket( xConnectedSocket );
            }

            debug_printf("Connection closed\n");
            debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
            debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

            vPortFree(audio_data);
            xConnected = pdFALSE;
            vTaskDelete( NULL );
        }

        vPortFree(audio_data);
    }
}

static void vlisten_for_mic_tcp_conn( void *arg )
{
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 1;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay(pdMS_TO_TICKS( 100 ));
    }

    /* Attempt to open the socket. */
    xListeningSocket = FreeRTOS_socket(
            FREERTOS_AF_INET,
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

    /* Set the listening port to appconfTHRUPUT_TEST_PORT. */
    xBindAddress.sin_port = FreeRTOS_htons( appconfQUEUE_TO_TCP_PORT );

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

        xTaskCreate( queue_to_tcp_sender, "q2tcp_send", portTASK_STACK_DEPTH(queue_to_tcp_sender), ( void * ) xConnectedSocket, uxTaskPriorityGet( NULL ), NULL );
    }
}

void queue_to_tcp_stream_create(QueueHandle_t input, UBaseType_t priority)
{
    queue_to_tcp = input;
    xTaskCreate( vlisten_for_mic_tcp_conn, "q2tcp_listen", portTASK_STACK_DEPTH(vlisten_for_mic_tcp_conn), NULL, priority, NULL );
}
