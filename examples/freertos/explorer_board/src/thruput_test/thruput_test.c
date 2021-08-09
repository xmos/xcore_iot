// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT THRUPUT_TEST
/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "thruput_test.h"

static void thruput_test_sender( void *arg )
{
    Socket_t xConnectedSocket = ( Socket_t ) arg;
    const TickType_t xSendTimeOut = pdMS_TO_TICKS( 5000 );
    const size_t xTotalLengthToSend = 100*1024*1024; /* 100 MiB */
    size_t xLenToSend;
    BaseType_t xMSS;
    BaseType_t xAlreadyTransmitted = 0, xBytesSent = 0;
    uint8_t snd_buf[ipconfigTCP_MSS * 4];

    BaseType_t xWaitForFullMSS = 1;

    for( int i = 0; i < sizeof(snd_buf); i++ )
    {
        snd_buf[i] = i & 0xFF;
    }

    FreeRTOS_setsockopt( xConnectedSocket,
                         0,
                         FREERTOS_SO_SNDTIMEO,
                         &xSendTimeOut,
                         sizeof( xSendTimeOut ) );

    /* Wait until we are sending a full MSS window. */
    FreeRTOS_setsockopt( xConnectedSocket,
                         0,
                         FREERTOS_SO_SET_FULL_SIZE,
                         &xWaitForFullMSS,
                         sizeof( xWaitForFullMSS ) );

    xMSS = FreeRTOS_mss( xConnectedSocket );

    debug_printf("Thruput test begin, mss is %d\n", xMSS);

    /* Keep sending until the entire buffer has been sent. */
    while( xAlreadyTransmitted < xTotalLengthToSend )
    {
        /* How many bytes are left to send? */
        xLenToSend = FreeRTOS_min_int32(xTotalLengthToSend - xAlreadyTransmitted, 4 * xMSS);

        if (xLenToSend < 4 * xMSS)
        {
            xWaitForFullMSS = 0;
            /* Ensure the last bit is sent if it's smaller than the MSS */
            FreeRTOS_setsockopt( xConnectedSocket,
                                 0,
                                 FREERTOS_SO_SET_FULL_SIZE,
                                 &xWaitForFullMSS,
                                 sizeof( xWaitForFullMSS ) );
        }

        xBytesSent = FreeRTOS_send(
                xConnectedSocket,
                /* The data being sent. */
                snd_buf,
                /* The remaining length of data to send. */
                xLenToSend,
                /* ulFlags. */
                0 );

        if( xBytesSent >= 0 )
        {
            /* Data was sent successfully. */
            xAlreadyTransmitted += xBytesSent;
        }
        else if ( xBytesSent != -pdFREERTOS_ERRNO_ENOSPC )
        {
            /* Error – break out of the loop for graceful socket close. */
            break;
        }
    }

    debug_printf("Thruput test complete, closing connection\n");
    /* Initiate graceful shutdown. */
    FreeRTOS_shutdown( xConnectedSocket, FREERTOS_SHUT_RDWR );
    while( FreeRTOS_recv( xConnectedSocket, snd_buf, sizeof(snd_buf), FREERTOS_MSG_DONTWAIT ) >= 0 )
    {
        vTaskDelay(pdMS_TO_TICKS( 100 ));
    }
    FreeRTOS_closesocket( xConnectedSocket );
    debug_printf("Thruput test connection closed\n");

    vTaskDelete( NULL );
}

static void vthruput_test( void *arg )
{
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 4;

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

    /* If FREERTOS_SO_RCVBUF or FREERTOS_SO_SNDBUF are to be used with
    FreeRTOS_setsockopt() to change the buffer sizes from their default then do
    it here!.  (see the FreeRTOS_setsockopt() documentation. */

    /* If ipconfigUSE_TCP_WIN is set to 1 and FREERTOS_SO_WIN_PROPERTIES is to
    be used with FreeRTOS_setsockopt() to change the sliding window size from
    its default then do it here! (see the FreeRTOS_setsockopt()
    documentation. */

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt( xListeningSocket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeOut,
                         sizeof( xReceiveTimeOut ) );

    /* Set the listening port to appconfTHRUPUT_TEST_PORT. */
    xBindAddress.sin_port = ( uint16_t ) appconfTHRUPUT_TEST_PORT;
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );

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

        xTaskCreate((TaskFunction_t) thruput_test_sender,
                    "thruput_test_sender",
                    portTASK_STACK_DEPTH(thruput_test_sender),
                    (void*) xConnectedSocket,
                    uxTaskPriorityGet( NULL ),
                    NULL );
    }
}

void thruput_test_create( UBaseType_t priority )
{
    xTaskCreate((TaskFunction_t)vthruput_test,
                "thruput_test",
                portTASK_STACK_DEPTH(vthruput_test),
                NULL,
                priority,
                NULL );
}
