/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "MQTTFreeRTOS.h"

#include "tls_support.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls_support.h"


#define MQTT_TASK_STACK_SIZE	( configSTACK_DEPTH_TYPE )( 420 )


int ThreadStart( Thread* thread, void (*fn)(void*), void* arg )
{
	BaseType_t xRetVal = pdFAIL;
	UBaseType_t uxTaskPriority = ( uxTaskPriorityGet( NULL ) > tskIDLE_PRIORITY )
								 ? uxTaskPriorityGet( NULL ) - 1
								 : tskIDLE_PRIORITY;

	xRetVal = xTaskCreate(fn, "MQTTTask", MQTT_TASK_STACK_SIZE, arg, uxTaskPriority, &thread->task);

	return ( int ) xRetVal;
}


void MutexInit( Mutex* mutex )
{
	mutex->sem = xSemaphoreCreateMutex();
}

int MutexLock( Mutex* mutex )
{
	return xSemaphoreTake( mutex->sem, portMAX_DELAY );
}

int MutexUnlock( Mutex* mutex )
{
	return xSemaphoreGive( mutex->sem );
}


void TimerCountdownMS( Timer* timer, unsigned int timeout_ms )
{
	timer->xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	vTaskSetTimeOutState( &timer->xTimeOut ); /* Record the time at which this function was entered. */
}


void TimerCountdown( Timer* timer, unsigned int timeout )
{
	TimerCountdownMS( timer, timeout * 1000 );
}


int TimerLeftMS( Timer* timer )
{
	xTaskCheckForTimeOut( &timer->xTimeOut, &timer->xTicksToWait ); /* updates xTicksToWait to the number left */
	return ( timer->xTicksToWait * portTICK_PERIOD_MS );
}


char TimerIsExpired( Timer* timer )
{
	return xTaskCheckForTimeOut( &timer->xTimeOut, &timer->xTicksToWait ) == pdTRUE;
}


void TimerInit( Timer* timer )
{
	timer->xTicksToWait = 0;
	memset( &timer->xTimeOut, '\0', sizeof( timer->xTimeOut ) );
}


int FreeRTOS_read( Network* n, unsigned char* buffer, int len, int timeout_ms )
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int recvLen = 0;

	vTaskSetTimeOutState( &xTimeOut ); /* Record the time at which this function was entered. */
	do
	{
		int rc = 0;

		FreeRTOS_setsockopt( n->my_socket, 0, FREERTOS_SO_RCVTIMEO, &xTicksToWait, sizeof( xTicksToWait ) );
		if( n->ssl_ctx != NULL )
		{
			rc = mbedtls_ssl_read( n->ssl_ctx, buffer + recvLen, len - recvLen );
			if( rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				/* TODO we must have caller close the ssl ctx */
				recvLen = rc;
				break;
			}
		}
		else
		{
			rc = FreeRTOS_recv( n->my_socket, buffer + recvLen, len - recvLen, 0 );
		}
		if( rc > 0 )
			recvLen += rc;
		else if( rc < 0 )
		{
			recvLen = rc;
			break;
		}
	} while( recvLen < len && xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE );

	return recvLen;
}


int FreeRTOS_write( Network* n, unsigned char* buffer, int len, int timeout_ms )
{
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	int sentLen = 0;

	vTaskSetTimeOutState( &xTimeOut ); /* Record the time at which this function was entered. */
	do
	{
		int rc = 0;

		FreeRTOS_setsockopt( n->my_socket, 0, FREERTOS_SO_RCVTIMEO, &xTicksToWait, sizeof( xTicksToWait ) );
		if( n->ssl_ctx != NULL )
		{
			rc = mbedtls_ssl_write( n->ssl_ctx, buffer + sentLen, len - sentLen );
			if( rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				/* TODO we must have caller close the ssl ctx */
				sentLen = rc;
				break;
			}
		}
		else
		{
			rc = FreeRTOS_send( n->my_socket, buffer + sentLen, len - sentLen, 0 );
		}
		if( rc > 0 )
			sentLen += rc;
		else if( rc < 0 )
		{
			sentLen = rc;
			break;
		}
	} while( sentLen < len && xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE );

	return sentLen;
}


void FreeRTOS_disconnect( Network* n )
{
	if( n->ssl_ctx == NULL )
	{
		FreeRTOS_closesocket( n->my_socket );
	}
}


void NetworkInit( Network* n)
{
	n->my_socket = NULL;
	n->mqttread = FreeRTOS_read;
	n->mqttwrite = FreeRTOS_write;
	n->disconnect = FreeRTOS_disconnect;
	n->ssl_ctx = NULL;
}


int NetworkConnect( Network* n, char* addr, int port )
{
	struct freertos_sockaddr sAddr;
	int retVal = -1;
	uint32_t ipAddress = 0;

	do
	{
		if( ( ipAddress = FreeRTOS_gethostbyname( addr ) ) == 0 )
		{
			debug_printf("failed to get hostname\n");
			break;
		}

		sAddr.sin_port = FreeRTOS_htons( port );
		sAddr.sin_addr = ipAddress;

		if( ( n->my_socket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP ) ) < 0 )
		{
			debug_printf("failed to get socket\n");
			break;
		}

		if( ( retVal = FreeRTOS_connect( n->my_socket, &sAddr, sizeof( sAddr ) ) ) < 0 )
		{
			FreeRTOS_closesocket( n->my_socket );
			debug_printf("failed to connect\n");
			break;
		}
	} while( 0 );

	return retVal;
}


int NetworkConnectIP( Network* n, uint32_t addr, int port )
{
	struct freertos_sockaddr sAddr;
	int retVal = -1;

	sAddr.sin_port = FreeRTOS_htons( port );
	sAddr.sin_addr = addr;

	do
	{
		if( ( n->my_socket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP ) ) < 0 )
		{
			debug_printf("failed to get socket\n");
			break;
		}

		if( ( retVal = FreeRTOS_connect( n->my_socket, &sAddr, sizeof( sAddr ) ) ) < 0 )
		{
			FreeRTOS_closesocket( n->my_socket );
			debug_printf("failed to connect\n");
			break;
		}
	} while( 0 );

	return retVal;
}
