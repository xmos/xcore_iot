// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "tls_support.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/debug.h"

/* App headers */
#include "app_conf.h"
#include "http_demo.h"
#include "http_parser.h"


typedef struct http_parser_cb
{
	Socket_t socket;
	void* buf;
	size_t buf_len;
} http_parser_cb_t;


int url_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("url cb %d %s\n", length, (char*)at);
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int msg_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("msg cb %d %s\n", length, (char*)at);
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int status_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("status cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int hdr_field_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("hdr field cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int hdr_val_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("hdr val cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int hdr_complete_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("hdr complete cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int body_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("body cb %d %s\n", length, (char*)at);
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int msg_complete_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("msg complete\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);
	http_parser_cb_t *cb_data = parser->data;
	Socket_t socket = ( Socket_t ) cb_data->socket;
#define retmsg "test\n"
	FreeRTOS_send( socket, retmsg, strlen(retmsg), 0);

	return 0;
}

int chunk_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("chunk cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

int chunk_complete_callback(http_parser* parser, const char *at, size_t length)
{
	debug_printf("chunk complete cb\n");
	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "http" ) );
	debug_printf("http free stack words: %d\n", free_stack_words);

	return 0;
}

void http_parser_thread( void *arg )
{
    Socket_t socket = ( Socket_t ) arg;
    int8_t *data;
    BaseType_t bytes_rx = 0;
    int data_length = 1024;

	http_parser_cb_t *cb_data = pvPortMalloc( sizeof( http_parser_cb_t ) );

	cb_data->socket = socket;

	http_parser *parser = pvPortMalloc( sizeof( http_parser ) );
	http_parser_init(parser, HTTP_BOTH);

	parser->data = cb_data;

	http_parser_settings *settings = pvPortMalloc( sizeof( http_parser_settings ) );
	http_parser_settings_init(settings);
	settings->on_url = (http_data_cb)url_callback;
	settings->on_message_begin = (http_cb)msg_callback;
	settings->on_status = (http_data_cb)status_callback;
	settings->on_header_field = (http_data_cb)hdr_field_callback;
	settings->on_header_value = (http_data_cb)hdr_val_callback;
	settings->on_headers_complete = (http_cb)hdr_complete_callback;
	settings->on_body = (http_data_cb)body_callback;
	settings->on_message_complete = (http_cb)msg_complete_callback;
	settings->on_chunk_header = (http_cb)chunk_callback;
	settings->on_chunk_complete = (http_cb)chunk_complete_callback;

	data = pvPortMalloc( sizeof(int8_t) * data_length );
	memset( data, 0x00, sizeof(int8_t) * data_length );

    for (;;) {
		bytes_rx = FreeRTOS_recv( socket, data, data_length, 0 );

		if( bytes_rx > 0 )
		{
			int bparsed;
			bparsed = http_parser_execute(parser, settings, (const char *) data, bytes_rx);
		}
		else
		{
			FreeRTOS_shutdown( socket, FREERTOS_SHUT_RDWR );

			char dummy;
			while( FreeRTOS_recv( socket, &dummy, 1, 0 ) >= 0 )
			{
				vTaskDelay(pdMS_TO_TICKS( 100 ));
			}

			FreeRTOS_closesocket( socket );

			debug_printf("Connection closed\n");
			debug_printf("Heap free: %d\n", xPortGetFreeHeapSize());
			debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());

			vPortFree(data);
			vPortFree(settings);
			vPortFree(parser);
			vPortFree(cb_data);

			vTaskDelete( NULL );
		}
    }
}

static void http_demo( void *arg )
{
	( void ) arg;

    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const TickType_t xSendTimeOut = pdMS_TO_TICKS( 1000 );
    const BaseType_t xBacklog = 1;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
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
    xBindAddress.sin_port = FreeRTOS_htons( 80 );

    /* Bind the socket to the port that the client RTOS task will send to. */
    FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

    /* Set the socket into a listening state so it can accept connections.
    The maximum number of simultaneous connections is limited to 20. */
    FreeRTOS_listen( xListeningSocket, xBacklog );

    debug_printf("http parser v%u.%u.%u\n",
    			(http_parser_version() >> 16) & 255,
				(http_parser_version() >>  8) & 255,
				(http_parser_version() >>  0) & 255 );

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

        xTaskCreate( http_parser_thread, "http", 150, ( void * ) xConnectedSocket, uxTaskPriorityGet( NULL ), NULL );
    }
}

void http_demo_create( UBaseType_t priority )
{
    xTaskCreate( http_demo, "http", portTASK_STACK_DEPTH( http_demo ), NULL , priority, NULL );
}
