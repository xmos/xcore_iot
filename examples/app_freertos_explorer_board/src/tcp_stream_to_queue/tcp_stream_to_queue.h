// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef TCP_STREAM_TO_QUEUE_H_
#define TCP_STREAM_TO_QUEUE_H_

#include "FreeRTOS_Sockets.h"

typedef struct tcp_to_queue_ctx
{
	QueueHandle_t queue;
	TickType_t rx_timeout;
	TickType_t tx_timeout;
	BaseType_t connected;
	size_t data_length;
	Socket_t socket;
	uint16_t port;
} tcp_to_queue_ctx_t;

typedef struct tcp_to_queue_ctx * tcp_to_queue_handle_t;

tcp_to_queue_handle_t tcp_to_queue_create( QueueHandle_t queue, uint16_t port, TickType_t rx_timeout,  TickType_t tx_timeout, size_t data_length );
BaseType_t is_tcp_to_queue_connected( tcp_to_queue_handle_t handle );
void tcp_stream_to_queue_create( tcp_to_queue_handle_t handle, UBaseType_t priority );

#endif /* TCP_STREAM_TO_QUEUE_H_ */
