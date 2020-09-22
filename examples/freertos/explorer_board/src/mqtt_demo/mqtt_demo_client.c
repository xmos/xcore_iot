// Copyright (c) 2020, XMOS Ltd, All rights reserved

#define DEBUG_UNIT MQTT_DEMO_CLIENT
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
#include "tls_support.h"
#include "MQTTClient.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "gpio_driver.h"

/* App headers */
#include "mqtt_demo_client.h"


#define MQTT_DEMO_CONNECT_STACK_SIZE 		800

const char* HOSTNAME = "localhost";

typedef struct net_conn_args
{
	Socket_t socket;
	mbedtls_ssl_context* ssl_ctx;
	TaskHandle_t connection_task;
} net_conn_args_t;

static soc_peripheral_t dev;
static uint32_t val;

void messageArrived(MessageData* data)
{
	val ^= 1;
	gpio_write_pin(dev, gpio_4C, 0, val);
	debug_printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}


static void mqtt_handler( void* arg )
{
	net_conn_args_t* args = ( net_conn_args_t* ) arg;
	MQTTClient client;
	Network network;
	unsigned char sendbuf[1024], readbuf[1024];
	int retval = 0;
	int	count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	dev = bitstream_gpio_devices[ BITSTREAM_GPIO_DEVICE_A ];
	val = 0;

	TaskHandle_t caller = args->connection_task;

	NetworkInit( &network );
	MQTTClientInit( &client, &network, 30000, sendbuf, sizeof( sendbuf ), readbuf, sizeof( readbuf ) );

	network.my_socket = args->socket;
	network.ssl_ctx = args->ssl_ctx;

    MQTTStartTask( &client );

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "FreeRTOS_demo";

	for( ;; )
	{
		if( ( retval = MQTTConnect( &client, &connectData ) ) != 0 )
		{
			debug_printf("Return code from MQTT connect is %d\n", retval);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			debug_printf("MQTT Connected\n");

			if( ( retval = MQTTSubscribe( &client, "echo/b", QOS1, messageArrived ) ) != 0 )
			{
				debug_printf("Return code from MQTT subscribe is %d\n", retval);
			}
			else
			{
				debug_printf("MQTT Subscribed\n");

				while( 1 )
				{
					MQTTMessage message;
					char payload[30] = "hello world\n";

					message.qos = 1;
					message.retained = 0;
					message.payload = payload;
					message.payloadlen = strlen(payload);

					debug_printf("Try to publish: %s\n", payload);
					if( ( retval = MQTTPublish( &client, "echo/a", &message ) ) != 0 )
					{
						debug_printf("Return code from MQTT publish is %d\n", retval);
						break;
					}
					else
					{
						debug_printf("Published %s\n", payload);
					}
					vTaskDelay( pdMS_TO_TICKS( 1000 ) );
				}

				/* cleanup and resume listener task */
				vTaskDelete( client.thread.task );
				vTaskResume( caller );
				vTaskDelete( NULL );
			}
		}
	}
}

static void mqtt_demo_connect( void* arg )
{
	( void ) arg;
	TaskHandle_t handler_task;
	int retval = 0;
	int tmpval = 0;
	struct freertos_sockaddr sAddr;
	int recv_timeout = pdMS_TO_TICKS( 5000 );
	int send_timeout = pdMS_TO_TICKS( 5000 );

	/* TODO make these args */
	sAddr.sin_port = FreeRTOS_htons( appconfMQTT_PORT );
	sAddr.sin_addr = FreeRTOS_inet_addr_quick(
						appconfMQTT_SERVER_IP_ADDR_OCTET_0,
						appconfMQTT_SERVER_IP_ADDR_OCTET_1,
						appconfMQTT_SERVER_IP_ADDR_OCTET_2,
						appconfMQTT_SERVER_IP_ADDR_OCTET_3 );

	Socket_t socket;
	/* These can be shared by multiple connections */
	mbedtls_x509_crt* cert = pvPortMalloc( sizeof( mbedtls_x509_crt ) );
	mbedtls_pk_context* prvkey = pvPortMalloc( sizeof( mbedtls_pk_context ) );
	mbedtls_x509_crt* ca = pvPortMalloc( sizeof( mbedtls_x509_crt ) );

	while( tls_platform_ready() == 0 )
	{
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
	}

	mbedtls_x509_crt_init( cert );
	mbedtls_pk_init( prvkey );
	mbedtls_x509_crt_init( ca );

	get_device_cert( cert );
	get_device_prvkey( prvkey );
	get_ca_cert( ca );

	/* These can be shared by multiple connections, if same credentials are used */
	mbedtls_ssl_config* ssl_conf  = pvPortMalloc( sizeof( mbedtls_ssl_config ) );

	mbedtls_ssl_config_init( ssl_conf );

	if( mbedtls_ssl_config_defaults( ssl_conf,
									 MBEDTLS_SSL_IS_CLIENT,
									 MBEDTLS_SSL_TRANSPORT_STREAM,
									 MBEDTLS_SSL_PRESET_DEFAULT )
			!= 0 )
	{
		configASSERT(0); /* Failed to initialize ssl conf with defaults */
	}

	if( mbedtls_ssl_conf_own_cert( ssl_conf, cert, prvkey ) != 0 )
	{
		configASSERT(0); /* Failed to initialize ssl conf with device cert and key */
	}

	mbedtls_ctr_drbg_context *drbg_ptr = get_shared_drbg_ctx();
	mbedtls_ssl_conf_rng( ssl_conf, mbedtls_ctr_drbg_random, drbg_ptr );

	mbedtls_ssl_conf_authmode( ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED );
	mbedtls_ssl_conf_ca_chain( ssl_conf, ca, NULL );

	/* SSL context is per connection */
	mbedtls_ssl_context* ssl_ctx = pvPortMalloc( sizeof( mbedtls_ssl_context ) );

	tls_ctx_t* tls_ctx = pvPortMalloc( sizeof( tls_ctx_t ) );

	for( ;; )
	{
		mbedtls_ssl_init( ssl_ctx );

		if( mbedtls_ssl_setup( ssl_ctx, ssl_conf ) != 0 )
		{
			configASSERT(0); /* Failed to initialize ssl context */
		}

		if( ( mbedtls_ssl_set_hostname( ssl_ctx, HOSTNAME ) ) != 0 )
		{
			configASSERT( 0 ); /* set hostname failed */
		}

		mbedtls_ssl_set_bio( ssl_ctx, tls_ctx, tls_send, tls_recv, NULL );

		while( FreeRTOS_IsNetworkUp() == pdFALSE )
		{
			vTaskDelay( pdMS_TO_TICKS( 100 ) );
		}

		socket = FreeRTOS_socket( FREERTOS_AF_INET,
								  FREERTOS_SOCK_STREAM,
								  FREERTOS_IPPROTO_TCP );

		/* Check the socket was created. */
		configASSERT( socket != FREERTOS_INVALID_SOCKET );

		/* Set time outs */
		FreeRTOS_setsockopt( socket,
							 0,
							 FREERTOS_SO_RCVTIMEO,
							 &recv_timeout,
							 sizeof( TickType_t ) );

		FreeRTOS_setsockopt( socket,
							 0,
							 FREERTOS_SO_SNDTIMEO,
							 &send_timeout,
							 sizeof( TickType_t ) );

		if(	FreeRTOS_connect( socket, &sAddr, sizeof( sAddr ) ) == 0 )
		{
			tls_ctx->socket = socket;
			/* attempt to handshake */
			while( ( tmpval = mbedtls_ssl_handshake( ssl_ctx ) ) != 0 )
			{
				if( tmpval != MBEDTLS_ERR_SSL_WANT_READ && tmpval != MBEDTLS_ERR_SSL_WANT_WRITE )
				{
					debug_printf( "mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int) -tmpval );
					break;
				}
			}

			/* verify peer cert */
			if( ( tmpval = mbedtls_ssl_get_verify_result( ssl_ctx ) ) != 0 )
			{
				debug_printf("failed to verify peer cert\n");
			}
			else
			{
				// run mqtt task now
				net_conn_args_t* handler_args = pvPortMalloc( sizeof( net_conn_args_t ) );

				handler_args->socket = socket;
				handler_args->ssl_ctx = ssl_ctx;
				handler_args->connection_task = xTaskGetCurrentTaskHandle();

				xTaskCreate( mqtt_handler, "mqtt", ( 1200 ), ( void * ) handler_args, uxTaskPriorityGet( NULL ) + 1, &handler_task );

				vTaskSuspend( NULL );
				vPortFree( handler_args );
			}

			mbedtls_ssl_close_notify( ssl_ctx );

			FreeRTOS_shutdown( socket, FREERTOS_SHUT_RDWR );

			char dummy;
			while( FreeRTOS_recv( socket, &dummy, 1, FREERTOS_MSG_DONTWAIT ) >= 0 )
			{
				vTaskDelay( pdMS_TO_TICKS( 100 ) );
			}
		}

		FreeRTOS_closesocket( socket );

		mbedtls_ssl_free( ssl_ctx );
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
	}

	vPortFree( tls_ctx );
	vPortFree( ssl_ctx );
	vPortFree( ssl_conf );
	vPortFree( cert );
	vPortFree( prvkey );
	vPortFree( ca );
}

void mqtt_demo_create( UBaseType_t priority )
{
    xTaskCreate( mqtt_demo_connect, "mqtt_demo", MQTT_DEMO_CONNECT_STACK_SIZE, ( void * ) NULL, priority, NULL );
}
