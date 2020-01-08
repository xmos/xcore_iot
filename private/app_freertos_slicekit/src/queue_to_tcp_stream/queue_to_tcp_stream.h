// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef QUEUE_TO_TCP_STREAM_H_
#define QUEUE_TO_TCP_STREAM_H_

typedef enum
{
    eSTATE_NETWORK_DOWN,
    eSTATE_GET_SOCKET,
    eSTATE_CONNECT_TO_SOCKET,
    eSTATE_CONNECTED,
    eSTATE_SHUTDOWN
} eTCP_queue_to_tcp_state;

void queue_to_tcp_stream_create(QueueHandle_t input, UBaseType_t priority);
BaseType_t is_queue_to_tcp_connected( void );

#endif /* QUEUE_TO_TCP_STREAM_H_ */
