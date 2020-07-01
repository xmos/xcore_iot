// Copyright (c) 2020, XMOS Ltd, All rights reserved

//#define DEBUG_UNIT MQTT_DEMO_CLIENT
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
#include "gpio_driver.h"

/* App headers */
#include "mqtt_demo_client.h"

#include "MQTTClient.h"

#define MQTT_SERVER_IP_ADDR_OCTET_0    10
#define MQTT_SERVER_IP_ADDR_OCTET_1    0
#define MQTT_SERVER_IP_ADDR_OCTET_2    0
#define MQTT_SERVER_IP_ADDR_OCTET_3    253
#define MQTT_PORT 1883

static soc_peripheral_t dev;
static uint32_t val;

void messageArrived(MessageData* data)
{
	val ^= 1;
	gpio_write_pin(dev, gpio_4C, 0, val);
	debug_printf("Message arrived on topic %s: %s\n", data->topicName->lenstring.data,
		data->message->payload);
}

static void mqtt_demo( void *arg )
{
	( void ) arg;
	MQTTClient client;
	Network network;
	unsigned char sendbuf[80], readbuf[80];
	int retval = 0;
	int	count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	dev = bitstream_gpio_devices[ BITSTREAM_GPIO_DEVICE_A ];
	val = 0;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }
	NetworkInit( &network );
	MQTTClientInit( &client, &network, 30000, sendbuf, sizeof( sendbuf ), readbuf, sizeof( readbuf ) );

	while( ( retval = NetworkConnectIP( &network,
									    FreeRTOS_inet_addr_quick(
										    MQTT_SERVER_IP_ADDR_OCTET_0,
										    MQTT_SERVER_IP_ADDR_OCTET_1,
										    MQTT_SERVER_IP_ADDR_OCTET_2,
										    MQTT_SERVER_IP_ADDR_OCTET_3 ),
									    MQTT_PORT ) )
			!= 0)
	{
		debug_printf("Return code from network connect is %d\n", retval);
	    vTaskDelay( pdMS_TO_TICKS( 1000 ) );
	}
	debug_printf("Connected\n");
    MQTTStartTask( &client );

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "FreeRTOS_demo";

	for( ;; )
	{
		if ((retval = MQTTConnect(&client, &connectData)) != 0)
		{
			debug_printf("Return code from MQTT connect is %d\n", retval);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		else
		{
			debug_printf("MQTT Connected\n");

			if ((retval = MQTTSubscribe(&client, "echo/b", QOS0, messageArrived)) != 0)
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
					if ((retval = MQTTPublish(&client, "echo/a", &message)) != 0)
					{
						debug_printf("Return code from MQTT publish is %d\n", retval);
					}
					else
					{
						debug_printf("Published %s\n", payload);
					}
					debug_printf("delay\n");
					vTaskDelay(pdMS_TO_TICKS(1000));
					debug_printf("delay done\n");
				}
			}
		}
	}
}

void mqtt_demo_create( UBaseType_t priority )
{
    xTaskCreate( mqtt_demo, "mqtt_demo", ( 1000 ), ( void * ) NULL, priority, NULL );
}
