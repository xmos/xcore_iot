// Copyright 2019 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef NETWORK_H_
#define NETWORK_H_

#include "FreeRTOS_Sockets.h"

/* Initalize FreeRTOS Plus TCP IP stack */
void initalize_FreeRTOS_IP( void );

/* Initalize WiFi */
void initalize_wifi( void );


#endif /* NETWORK_H_ */
