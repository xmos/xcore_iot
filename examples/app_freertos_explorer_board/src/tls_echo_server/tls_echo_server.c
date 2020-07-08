// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"
#include "tls_support.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/debug.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"

/* App headers */
#include "tls_echo_server.h"
#include "app_conf.h"

const char* CHECK_HOSTNAME = "demodevice";

#define TLS_ECHO_SERVER_PORT				7777
#define TLS_ECHO_SERVER_TASK_STACKSIZE		950 + 1000

#define RECV_BUF_SIZE ( 100 )

#ifdef MBEDTLS_DEBUG_C
static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    const char *p, *basename;

    /* Extract basename from file */
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;

	rtos_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
	rtos_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "echosrv" ) );
	debug_printf("echosrv free stack words: %d\n", free_stack_words);

    debug_printf( "%s:%04d: |%d| %s", basename, line, level, str );
}
#endif


static void tls_echo_server( void *arg )
{
    Socket_t socket = ( Socket_t )arg;
    int tmpval;

	/* Get shared deterministic random bit generator */
	mbedtls_ctr_drbg_context *drbg_ptr = get_shared_drbg_ctx();

	/* These can be shared by multiple connections */
	mbedtls_pk_context prvkey;
	mbedtls_x509_crt ca;

	mbedtls_pk_init( &prvkey );
	mbedtls_x509_crt_init( &ca );

	if( get_key( &prvkey, "/flash/server/key.pem" ) == pdFAIL )
	{
		configASSERT(0);
	}
	if( get_cert( &ca, "/flash/server/ca.pem" ) == pdFAIL )
	{
		configASSERT(0);
	}

	/* These can be shared by multiple connections, if same credentials are used */
	mbedtls_ssl_config ssl_conf;

	mbedtls_ssl_config_init( &ssl_conf );

#ifdef MBEDTLS_DEBUG_C
	/* Add debug function to the conf */
	mbedtls_ssl_conf_dbg( &ssl_conf, my_debug, NULL );
	mbedtls_debug_set_threshold(4);
#endif

	if( ( tmpval = mbedtls_ssl_config_defaults( &ssl_conf,
												MBEDTLS_SSL_IS_SERVER,
												MBEDTLS_SSL_TRANSPORT_STREAM,
												MBEDTLS_SSL_PRESET_DEFAULT ) )
			!= 0 )
	{
		debug_printf( "mbedtls_ssl_config_defaults returned %d\n\n", tmpval );
		mbedtls_x509_crt_free( &ca );
		mbedtls_pk_free( &prvkey );
		mbedtls_ssl_config_free( &ssl_conf );

		FreeRTOS_closesocket( socket );
	    vTaskDelete( NULL );
	}

	mbedtls_ssl_conf_authmode( &ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED );
	mbedtls_ssl_conf_own_cert( &ssl_conf, &ca, &prvkey );
	mbedtls_ssl_conf_ca_chain( &ssl_conf, &ca, NULL );
	mbedtls_ssl_conf_rng( &ssl_conf, mbedtls_ctr_drbg_random, drbg_ptr );

	unsigned char* recv_buf;
	unsigned int recv_len = RECV_BUF_SIZE;

	recv_buf = ( unsigned char* )pvPortMalloc( sizeof( unsigned char ) * recv_len );

	if( recv_buf == NULL )
	{
		mbedtls_x509_crt_free( &ca );
		mbedtls_pk_free( &prvkey );
		mbedtls_ssl_config_free( &ssl_conf );

		FreeRTOS_closesocket( socket );
	    vTaskDelete( NULL );
	}

	tls_ctx_t tls_ctx;
	tls_ctx.socket = socket;
	tls_ctx.flags = 0;

	/* SSL context is per connection */
	mbedtls_ssl_context ssl_ctx;

	mbedtls_ssl_init( &ssl_ctx );

	if( ( tmpval = mbedtls_ssl_setup( &ssl_ctx, &ssl_conf ) ) != 0 )
	{
		debug_printf( "mbedtls_ssl_setup returned %d\n\n", tmpval );
	}
	else
	{
		if( ( tmpval = mbedtls_ssl_set_hostname( &ssl_ctx, CHECK_HOSTNAME ) ) != 0 )
		{
			debug_printf( "mbedtls_ssl_set_hostname returned %d\n\n", tmpval );
			configASSERT( 0 );
		}

		mbedtls_ssl_set_bio( &ssl_ctx, &tls_ctx, tls_send, tls_recv, NULL );

		/* attempt to handshake */
		while( ( tmpval = mbedtls_ssl_handshake( &ssl_ctx ) ) != 0 )
		{
			if( tmpval != MBEDTLS_ERR_SSL_WANT_READ && tmpval != MBEDTLS_ERR_SSL_WANT_WRITE )
			{
				debug_printf( "mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int) -tmpval );
				break;
			}
		}

		int exit = 0;
		do
		{
			do
			{
				memset( recv_buf, 0, sizeof( sizeof( unsigned char ) * recv_len ) );
				tmpval = mbedtls_ssl_read( &ssl_ctx, recv_buf, recv_len );

				if( tmpval == MBEDTLS_ERR_SSL_WANT_READ || tmpval == MBEDTLS_ERR_SSL_WANT_WRITE )
					continue;

				if( ( tmpval == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY ) || ( tmpval <= 0 ) )
				{
					exit = 1;
					debug_printf( "mbedtls_ssl_read returned %d\n\n", tmpval );
					break;
				}

				debug_printf( " %d bytes read\n%s\n", tmpval, (char *) recv_buf );
			}
			while( 0 );

			while( ( tmpval = mbedtls_ssl_write( &ssl_ctx, recv_buf, tmpval ) ) <= 0 )
			{
				if( tmpval != MBEDTLS_ERR_SSL_WANT_READ && tmpval != MBEDTLS_ERR_SSL_WANT_WRITE )
				{
					debug_printf( "mbedtls_ssl_write returned %d\n\n", tmpval );
					exit = 1;
					break;
				}
			}

			debug_printf( "%d bytes written\n%s\n", tmpval, (char *) recv_buf );

			if( FreeRTOS_issocketconnected( socket ) == pdFALSE )
			{
				exit = 1;
			}

			debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
			debug_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

			int free_stack_words = uxTaskGetStackHighWaterMark( NULL );
			debug_printf("tls_echo_receiver free stack words: %d\n", free_stack_words);
		} while( exit == 0 );

		mbedtls_ssl_close_notify( &ssl_ctx );
	}

	FreeRTOS_shutdown( socket, FREERTOS_SHUT_RDWR );

	while( FreeRTOS_recv( socket, recv_buf, sizeof( unsigned char ) * recv_len , FREERTOS_MSG_DONTWAIT ) >= 0 )
	{
		vTaskDelay(pdMS_TO_TICKS( 100 ));
	}

	FreeRTOS_closesocket( socket );
	mbedtls_ssl_free( &ssl_ctx );
	mbedtls_x509_crt_free( &ca );
	mbedtls_pk_free( &prvkey );
	mbedtls_ssl_config_free( &ssl_conf );
	vPortFree( recv_buf );

    vTaskDelete( NULL );
}

static void tls_echo_receiver( void *arg )
{
	( void ) arg;

    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const TickType_t xSendTimeOut = pdMS_TO_TICKS( 5000 );
    const BaseType_t xBacklog = 1;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }
	while( tls_platform_ready() == 0 )
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
    xBindAddress.sin_port = FreeRTOS_htons( TLS_ECHO_SERVER_PORT );

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

    	xTaskCreate( tls_echo_server, "echosrv", TLS_ECHO_SERVER_TASK_STACKSIZE, ( void * ) xConnectedSocket, uxTaskPriorityGet( NULL ), NULL );
    }
}

void tls_echo_server_create( UBaseType_t priority )
{
    xTaskCreate( tls_echo_receiver, "tls_echo_rx", portTASK_STACK_DEPTH( tls_echo_receiver ), NULL , priority, NULL );
}
