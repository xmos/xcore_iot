// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef QUEUE_TO_TCP_STREAM_H_
#define QUEUE_TO_TCP_STREAM_H_

#include "FreeRTOS_Sockets.h"

typedef struct queue_to_tcp_ctx
{
	QueueHandle_t queue;
	TickType_t rx_timeout;
	TickType_t tx_timeout;
	BaseType_t connected;
	size_t data_length;
	Socket_t socket;
	uint16_t port;
} queue_to_tcp_ctx_t;

typedef struct queue_to_tcp_ctx * queue_to_tcp_handle_t;

queue_to_tcp_handle_t queue_to_tcp_create( QueueHandle_t queue, uint16_t port, TickType_t rx_timeout,  TickType_t tx_timeout, size_t data_length );
BaseType_t is_queue_to_tcp_connected( queue_to_tcp_handle_t handle );
void queue_to_tcp_stream_create( queue_to_tcp_handle_t handle, UBaseType_t priority );

#endif /* QUEUE_TO_TCP_STREAM_H_ */
