/*
 * queue_to_tcp_stream.c
 *
 *  Created on: Oct 23, 2019
 *      Author: jmccarthy
 */

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

struct freertos_sockaddr xServerAddr;

static Socket_t xSocket;
static eTCP_queue_to_tcp_state eCurrentState = eSTATE_NETWORK_DOWN;

void vQueuetoTCPChangeHost(int port, int addr0, int addr1, int addr2, int addr3)
{
    xServerAddr.sin_port = FreeRTOS_htons(port);
    xServerAddr.sin_addr = FreeRTOS_inet_addr_quick(addr0, addr1, addr2, addr3);
    eCurrentState = eSTATE_SHUTDOWN;
}

static BaseType_t vCreateTCPClientSocket( void )
{
static const TickType_t xTXTimeOut = pdMS_TO_TICKS( 60000 );
static const TickType_t xRXTimeOut = pdMS_TO_TICKS( 2000 );
BaseType_t xRetVal;
BaseType_t xSocketStatus;

    /* Attempt to open the socket. */
    xSocket = FreeRTOS_socket( FREERTOS_AF_INET,
                               FREERTOS_SOCK_STREAM,
                               FREERTOS_IPPROTO_TCP );

    /* Check the socket was created. */
    if( xSocket == FREERTOS_INVALID_SOCKET )
    {
        xRetVal = pdFAIL;
    }
    else
    {
        do
        {
            /* Set send and receive time outs. */
            /* TODO, fix recursion for stacksize calc */
            xSocketStatus = FreeRTOS_setsockopt( xSocket,
                                                 0,
                                                 FREERTOS_SO_RCVTIMEO,
                                                 &xRXTimeOut,
                                                 sizeof( xRXTimeOut ) );

            if( xSocketStatus != pdFREERTOS_ERRNO_NONE )
            {
                break;
            }

            xSocketStatus = FreeRTOS_setsockopt( xSocket,
                                                 0,
                                                 FREERTOS_SO_SNDTIMEO,
                                                 &xTXTimeOut,
                                                 sizeof( xTXTimeOut ) );

            if( xSocketStatus != pdFREERTOS_ERRNO_NONE )
            {
                break;
            }

            xSocketStatus = FreeRTOS_bind( xSocket, &xServerAddr, sizeof( xServerAddr ) );

            if( xSocketStatus != pdFREERTOS_ERRNO_NONE )
            {
                break;
            }

        } while (0);

        if( xSocketStatus != pdFREERTOS_ERRNO_NONE )
        {
            FreeRTOS_closesocket( xSocket );
            xRetVal = pdFAIL;
        }
        else
        {
            xRetVal = pdPASS;
        }
    }
    return xRetVal;
}


void vTCP_queue_to_tcp(void *arg)
{
    QueueHandle_t input_queue = arg;
    BaseType_t xSent;
    BaseType_t xRetry = 0;

    for (;;) {
        int32_t *audio_data;

        xQueueReceive(input_queue, &audio_data, portMAX_DELAY);

        switch( eCurrentState )
        {
        case eSTATE_NETWORK_DOWN:
            if( FreeRTOS_IsNetworkUp() == pdFALSE )
            {
                vTaskDelay( pdMS_TO_TICKS( appconfNETWORK_STATUS_CHECK_INTERVAL_MS ) );
                break;
            }
            else
            {
                eCurrentState = eSTATE_GET_SOCKET;
                /* and fall through */
            }
        case eSTATE_GET_SOCKET:
            if( vCreateTCPClientSocket() == pdFAIL )
            {
                vTaskDelay( pdMS_TO_TICKS( appconfNETWORK_GET_SOCKET_INTERVAL_MS ) );
                break;
            }
            else
            {
                eCurrentState = eSTATE_CONNECT_TO_SOCKET;
                /* and fall through */
            }
        case eSTATE_CONNECT_TO_SOCKET: {
            BaseType_t ret;

            if( ( ret = FreeRTOS_connect( xSocket, &xServerAddr, sizeof( xServerAddr ) ) ) != pdFREERTOS_ERRNO_NONE )
            {
                if( xRetry >= appconfNETWORK_CONNECT_RETRY_LIMIT )
                {
                    debug_printf("Connect failed, resetting\n");
                    FreeRTOS_closesocket( xSocket );
                    eCurrentState = eSTATE_NETWORK_DOWN;
                    xRetry = 0;
                }
                else
                {
                    debug_printf("Connect failed %d, will retry\n", ret);
                    debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                    debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
                    xRetry++;
                }
                vTaskDelay( pdMS_TO_TICKS( appconfNETWORK_CONNECT_RETRY_INTERVAL_MS ) );
                break;
            }
            else
            {
                debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
                xRetry = 0;
                eCurrentState = eSTATE_CONNECTED;
                /* and fall through */
            }
        }
        case eSTATE_CONNECTED:
            xSent = FreeRTOS_send( xSocket,
                                   audio_data,
                                   sizeof(int32_t) * appconfMIC_FRAME_LENGTH,
                                   0
                                   );

            if( xSent != ( sizeof(int32_t) * appconfMIC_FRAME_LENGTH ) )
            {
                debug_printf("Send to TCP lost\n");

                if( FreeRTOS_issocketconnected( xSocket ) == pdFALSE )
                {
                    debug_printf("Remote disconnected, try to reconnect\n");
                    eCurrentState = eSTATE_CONNECT_TO_SOCKET;
                }
                else
                {
                    debug_printf("Connection broken, shutting down connection\n");

                    debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                    debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

                    FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
                    char dummy;
                    while( FreeRTOS_recv( xSocket, &dummy, 1, 0 ) >= 0 );

                    configASSERT( FreeRTOS_issocketconnected( xSocket ) == pdFALSE );
                    FreeRTOS_closesocket( xSocket );
                    eCurrentState = eSTATE_NETWORK_DOWN;
                }

                debug_printf("Connection closed\n");
                debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

                xRetry = 0;
            } else {
                if (xRetry++ == 100) {
                    debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
                    debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
                    xRetry = 0;
                }
            }
            break;
        case eSTATE_SHUTDOWN:
            debug_printf("Shutting down connection\n");

            FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );
            char dummy;
            while( FreeRTOS_recv( xSocket, &dummy, 1, 0 ) >= 0 );

            configASSERT( FreeRTOS_issocketconnected( xSocket ) == pdFALSE );
            FreeRTOS_closesocket( xSocket );
            eCurrentState = eSTATE_GET_SOCKET;
            break;

        default:
            configASSERT(0);    /* unhandled case */
            break;
        }

        vPortFree(audio_data);
    }
}

void queue_to_tcp_stream_create(QueueHandle_t input, UBaseType_t priority)
{
    xServerAddr.sin_port = FreeRTOS_htons( appconfQUEUE_TO_TCP_PORT );
    xServerAddr.sin_addr = FreeRTOS_inet_addr_quick( appconfQUEUE_TO_TCP_ADDR0,
                                                     appconfQUEUE_TO_TCP_ADDR1,
                                                     appconfQUEUE_TO_TCP_ADDR2,
                                                     appconfQUEUE_TO_TCP_ADDR3 );

    xTaskCreate( vTCP_queue_to_tcp, "queue_to_tcp", portTASK_STACK_DEPTH(vTCP_queue_to_tcp), input, priority, NULL );
}
