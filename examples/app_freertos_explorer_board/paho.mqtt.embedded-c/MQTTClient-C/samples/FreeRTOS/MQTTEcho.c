/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#define MQTT_TASK 1
#include "MQTTClient.h"

#define MQTT_SERVER_IP_ADDR_OCTET_0    10
#define MQTT_SERVER_IP_ADDR_OCTET_1    0
#define MQTT_SERVER_IP_ADDR_OCTET_2    0
#define MQTT_SERVER_IP_ADDR_OCTET_3    253
#define MQTT_PORT 1883

void messageArrived(MessageData* data)
{
	debug_printf("Message arrived on topic %s: %s\n", data->topicName->lenstring.data,
		data->message->payload);
}

static void prvMQTTEchoTask(void *pvParameters)
{
	MQTTClient client;
	Network network;
	unsigned char sendbuf[80], readbuf[80];
	int rc = 0, 
		count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    while( FreeRTOS_IsNetworkUp() == pdFALSE )
    {
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }

	pvParameters = 0;
	NetworkInit(&network);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	while ((rc = NetworkConnectIP( &network,
			FreeRTOS_inet_addr_quick( MQTT_SERVER_IP_ADDR_OCTET_0,
									  MQTT_SERVER_IP_ADDR_OCTET_1,
									  MQTT_SERVER_IP_ADDR_OCTET_2,
									  MQTT_SERVER_IP_ADDR_OCTET_3 ),
									  MQTT_PORT)) != 0)
	{
		debug_printf("Return code from network connect is %d\n", rc);
	    vTaskDelay( pdMS_TO_TICKS( 1000 ) );
	}

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		debug_printf("Return code from start tasks is %d\n", rc);
#endif

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "FreeRTOS_sample";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		debug_printf("Return code from MQTT connect is %d\n", rc);
	else
		debug_printf("MQTT Connected\n");

	if ((rc = MQTTSubscribe(&client, "echo/#", 2, messageArrived)) != 0)
		debug_printf("Return code from MQTT subscribe is %d\n", rc);
	else
		debug_printf("MQTT Subscribed\n");

	MQTTMessage message;
	char payload[30] = "hello world\n";

	message.qos = 1;
	message.retained = 0;
	message.payload = payload;
	message.payloadlen = strlen(payload);

	while (++count)
	{
		debug_printf("Try to publish: %s\n", payload);
		if ((rc = MQTTPublish(&client, "echo/a", &message)) != 0)
			debug_printf("Return code from MQTT publish is %d\n", rc);
		else
		{
			debug_printf("Published %s\n", payload);
		}

		vTaskDelay(pdMS_TO_TICKS(1000));

#if !defined(MQTT_TASK)
		debug_printf("Hi\n");
		if ((rc = MQTTYield(&client, 1000)) != 0)
			debug_printf("Return code from yield is %d\n", rc);
#endif
	}
	/* do not return */
}


void vStartMQTTTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	BaseType_t x = 0L;

	xTaskCreate(prvMQTTEchoTask,	/* The function that implements the task. */
			"MQTTEcho0",			/* Just a text name for the task to aid debugging. */
			usTaskStackSize,	/* The stack size is defined in FreeRTOSIPConfig.h. */
			(void *)x,		/* The task parameter, not used in this case. */
			uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
			NULL);				/* The task handle is not used. */
}
/*-----------------------------------------------------------*/


