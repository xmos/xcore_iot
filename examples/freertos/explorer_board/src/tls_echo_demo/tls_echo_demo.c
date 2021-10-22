// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

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
#include "tls_echo_demo/tls_echo_demo.h"

const char* DEMO_HOSTNAME = "localhost";

typedef struct echo_tcp_ctx
{
	TickType_t rx_timeout;
	TickType_t tx_timeout;
	BaseType_t connected;
	size_t data_length;
	uint16_t port;
} echo_tcp_ctx_t;

typedef struct echo_tcp_ctx * echo_tcp_handle_t;

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

	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "echo" ) );
	debug_printf("tls_echo_receiver free stack words: %d\n", free_stack_words);

    debug_printf( "%s:%04d: |%d| %s", basename, line, level, str );
}
#endif

static void tls_echo_receiver( void *arg )
{
	echo_tcp_handle_t handle = ( echo_tcp_handle_t ) arg;
	Socket_t socket;
	int tmpval;

    struct freertos_sockaddr xBindAddress;

    /* Set the listening port */
    xBindAddress.sin_addr = FreeRTOS_inet_addr_quick( appconfECHO_IP_ADDR_OCTET_0,
                                                      appconfECHO_IP_ADDR_OCTET_1,
                                                      appconfECHO_IP_ADDR_OCTET_2,
                                                      appconfECHO_IP_ADDR_OCTET_3 );
    xBindAddress.sin_port = FreeRTOS_htons( handle->port );

	/* Get shared deterministic random bit generator */
	mbedtls_ctr_drbg_context *drbg_ptr = get_shared_drbg_ctx();

	/* These can be shared by multiple connections */
	mbedtls_x509_crt cert;
	mbedtls_pk_context prvkey;
	mbedtls_x509_crt ca;

	mbedtls_x509_crt_init( &cert );
	mbedtls_pk_init( &prvkey );
	mbedtls_x509_crt_init( &ca );

	get_device_cert( &cert );
	get_device_prvkey( &prvkey );
	get_ca_cert( &ca );

	/* These can be shared by multiple connections, if same credentials are used */
	mbedtls_ssl_config ssl_conf;

	mbedtls_ssl_config_init( &ssl_conf );

#ifdef MBEDTLS_DEBUG_C
	/* Add debug function to the conf */
	mbedtls_ssl_conf_dbg( &ssl_conf, my_debug, NULL );
	mbedtls_debug_set_threshold(1);
#endif

	if( ( tmpval = mbedtls_ssl_config_defaults( &ssl_conf,
												MBEDTLS_SSL_IS_CLIENT,
												MBEDTLS_SSL_TRANSPORT_STREAM,
												MBEDTLS_SSL_PRESET_DEFAULT ) )
			!= 0 )
	{
		debug_printf( "mbedtls_ssl_config_defaults returned %d\n\n", tmpval );
		mbedtls_x509_crt_free( &ca );
		mbedtls_x509_crt_free( &cert );
		mbedtls_pk_free( &prvkey );
		mbedtls_ssl_config_free( &ssl_conf );
    	vPortFree( handle );
	    vTaskDelete( NULL );
	}

	mbedtls_ssl_conf_authmode( &ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED );
	mbedtls_ssl_conf_ca_chain( &ssl_conf, &ca, NULL );
	mbedtls_ssl_conf_rng( &ssl_conf, mbedtls_ctr_drbg_random, drbg_ptr );

	unsigned char buf[] = "HELLO WORLD\n";
	unsigned int len = strlen( (char*)buf );

	unsigned char *recv_buf;
	unsigned int recv_len = len;
	recv_buf = ( unsigned char* )pvPortMalloc( sizeof( unsigned char ) * recv_len );

	if( recv_buf == NULL )
	{
		mbedtls_x509_crt_free( &ca );
		mbedtls_x509_crt_free( &cert );
		mbedtls_pk_free( &prvkey );
		mbedtls_ssl_config_free( &ssl_conf );
    	vPortFree( handle );
	    vTaskDelete( NULL );
	}

    while( 1 )
    {
		socket = FreeRTOS_socket( FREERTOS_AF_INET,
								  FREERTOS_SOCK_STREAM,
								  FREERTOS_IPPROTO_TCP );
		/* Check the socket was created. */
		configASSERT( socket != FREERTOS_INVALID_SOCKET );

		/* Set time outs */
		FreeRTOS_setsockopt( socket,
							 0,
							 FREERTOS_SO_RCVTIMEO,
							 &handle->rx_timeout,
							 sizeof( TickType_t ) );

		FreeRTOS_setsockopt( socket,
							 0,
							 FREERTOS_SO_SNDTIMEO,
							 &handle->tx_timeout,
							 sizeof( TickType_t ) );

		/* SSL context is per connection */
		mbedtls_ssl_context ssl_ctx;

		mbedtls_ssl_init( &ssl_ctx );

		if(	FreeRTOS_connect( socket, &xBindAddress, sizeof( xBindAddress ) ) == 0 )
		{
			tls_ctx_t tls_ctx;
			tls_ctx.socket = socket;
			tls_ctx.flags = 0;

			if( ( tmpval = mbedtls_ssl_setup( &ssl_ctx, &ssl_conf ) ) != 0 )
			{
				debug_printf( "mbedtls_ssl_setup returned %d\n\n", tmpval );
			}
			else
			{
				if( ( tmpval = mbedtls_ssl_set_hostname( &ssl_ctx, DEMO_HOSTNAME ) ) != 0 )
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

				/* verify peer cert */
				if( ( tmpval = mbedtls_ssl_get_verify_result( &ssl_ctx ) ) != 0 )
				{
					debug_printf("failed to verify peer cert\n");
				}
				else
				{
					int exit = 0;
					do
					{
						while( ( tmpval = mbedtls_ssl_write( &ssl_ctx, (unsigned char *)&buf, len ) ) <= 0 )
						{
							if( tmpval != MBEDTLS_ERR_SSL_WANT_READ && tmpval != MBEDTLS_ERR_SSL_WANT_WRITE )
							{
								debug_printf( "mbedtls_ssl_write returned %d\n\n", tmpval );
								exit = 1;
								break;
							}
						}

						debug_printf( "%d bytes written\n%s\n", tmpval, (char *) buf );

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

							recv_len = tmpval;
							debug_printf( " %d bytes read\n%s\n", recv_len, (char *) recv_buf );
						}
						while( 0 );

						if( FreeRTOS_issocketconnected( socket ) == pdFALSE )
						{
							exit = 1;
						}
						if( exit == 0 )
						{
							vTaskDelay( pdMS_TO_TICKS( 1000 ) );
						}
					} while( exit == 0 );

					mbedtls_ssl_close_notify( &ssl_ctx );
				}
			}

			FreeRTOS_shutdown( socket, FREERTOS_SHUT_RDWR );
			while( FreeRTOS_recv( socket, recv_buf, sizeof( unsigned char ) * recv_len , FREERTOS_MSG_DONTWAIT ) >= 0 )
			{
				vTaskDelay(pdMS_TO_TICKS( 100 ));
			}
		}

		FreeRTOS_closesocket( socket );

		mbedtls_ssl_free( &ssl_ctx );

		vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }

	mbedtls_x509_crt_free( &ca );
	mbedtls_x509_crt_free( &cert );
	mbedtls_pk_free( &prvkey );
	mbedtls_ssl_config_free( &ssl_conf );

	vPortFree( recv_buf );
	vPortFree( handle );

    vTaskDelete( NULL );
}

static void tls_echo( void *arg )
{
	( void ) arg;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }

	while( tls_platform_ready() == 0 )
	{
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
	}

	echo_tcp_handle_t handle;

	handle = ( echo_tcp_handle_t ) pvPortMalloc( sizeof( echo_tcp_ctx_t ) );

	if( handle == NULL )
	{
		configASSERT(0);
	}

	handle->port = appconfECHO_PORT;
    handle->rx_timeout = pdMS_TO_TICKS( 5000 );
    handle->tx_timeout = pdMS_TO_TICKS( 5000 );

	xTaskCreate( tls_echo_receiver, "echo", 950+1000, ( void * ) handle, uxTaskPriorityGet( NULL ), NULL );

#if 0
    for( ;; )
    {
    	debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
    	debug_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

    	int free_stack_words = uxTaskGetStackHighWaterMark( xTaskGetHandle( "echo" ) );
    	debug_printf("tls_echo_receiver free stack words: %d\n", free_stack_words);
    	vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
#else
    vTaskDelete( NULL );
#endif
}

void tls_echo_demo_create( UBaseType_t priority )
{
    xTaskCreate( tls_echo, "tls_echo", portTASK_STACK_DEPTH( tls_echo ), NULL , priority, NULL );
}
