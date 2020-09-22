// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef NETWORK_H_
#define NETWORK_H_

#include "FreeRTOS_Sockets.h"

/* Initalize FreeRTOS Plus TCP IP stack */
void initalize_FreeRTOS_IP( void );

/* Initalize WiFi */
void initalize_wifi( void );


#endif /* NETWORK_H_ */
