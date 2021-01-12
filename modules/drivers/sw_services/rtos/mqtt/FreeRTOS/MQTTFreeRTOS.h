/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(MQTTFreeRTOS_H)
#define MQTTFreeRTOS_H

#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"
#include "semphr.h"
#include "task.h"

#include "mbedtls_support.h"
#include "mbedtls/ssl.h"

/**
 * Timer abstraction type for MQTT handler
 */
typedef struct Timer
{
	TickType_t xTicksToWait;	/**< Ticks to wait */
	TimeOut_t xTimeOut;			/**< Timeout */
} Timer;

typedef struct Network Network;

/**
 * Network abstraction type for MQTT handler
 */
struct Network
{
	xSocket_t my_socket;									/**< FreeRTOS socket handle */
	int (*mqttread) (Network*, unsigned char*, int, int);	/**< Network read function pointer */
	int (*mqttwrite) (Network*, unsigned char*, int, int);	/**< Network send function pointer */
	void (*disconnect) (Network*);							/**< Network disconnect function pointer */
	mbedtls_ssl_context* ssl_ctx;							/**< TLS context */
};

/**
 * Create a new timer
 *
 * \param[in/out] timer  	  Timer pointer
 */
void TimerInit( Timer* );

/**
 * Check if timer is expired
 *
 * \param[in]     timer  	  Timer pointer
 *
 * \returns 	  pdTRUE 	  If timer is expired
 * 				  pdFALSE	  Otherwise
 */
char TimerIsExpired( Timer* );

/**
 * Set timer to countdown
 *
 * \param[in]     timer  	  Timer pointer
 * \param[in]     timeout  	  Time in milliseconds
 */
void TimerCountdownMS( Timer*, unsigned int );

/**
 * Set timer to countdown
 *
 * \param[in]     timer  	  Timer pointer
 * \param[in]     timeout  	  Time in seconds
 */
void TimerCountdown( Timer*, unsigned int );

/**
 * Check remaining time on timer
 *
 * \param[in]     timer  	  Timer pointer
 *
 * \returns 	  Remaining time in milliseconds
 */
int TimerLeftMS( Timer* );

/**
 * Mutex abstraction type for MQTT handler
 */
typedef struct Mutex
{
	SemaphoreHandle_t sem;	/**< FreeRTOS semaphore handle */
} Mutex;

/**
 * Initialize a mutex
 *
 * \param[in/out] mutex  	  Mutex pointer
 */
void MutexInit( Mutex* );

/**
 * Lock the mutex
 *
 * \param[in]     mutex  	  Mutex pointer
 *
 * \returns 	  pdTRUE 	  On success
 * 				  pdFALSE	  Otherwise
 */
int MutexLock( Mutex* );

/**
 * Unlock the mutex
 *
 * \param[in]     mutex  	  Mutex pointer
 *
 * \returns 	  pdTRUE 	  On success
 * 				  pdFALSE	  Otherwise
 */
int MutexUnlock( Mutex* );

/**
 * Thread abstraction type for MQTT handler
 */
typedef struct Thread
{
	TaskHandle_t task;	/**< FreeRTOS Task handle */
} Thread;

/**
 * Starts an MQTT session management thread
 *
 * \param[in/out] thread	  Thread pointer
 * \param[in] 	  fn	  	  Pointer to task function
 * \param[in] 	  arg		  Pointer to task arguments
 */
int ThreadStart( Thread*, void (*fn)(void*), void* arg );

/**
 * Performs a network read
 *
 * \param[in]     n		      Network pointer
 * \param[in/out] buffer	  Pointer to buffer to read into from network
 * \param[in] 	  len		  Number of bytes to be read
 * \param[in] 	  timeout	  Read timeout in ms
 */
int FreeRTOS_read( Network*, unsigned char*, int, int );

/**
 * Performs a network write
 *
 * \param[in]     n		     Network pointer
 * \param[in/out] buffer	 Pointer to buffer to write to network
 * \param[in] 	  len		 Number of bytes to be written
 * \param[in] 	  timeout	 Send timeout in ms
 *
 * \returns		  Number of bytes written
 */
int FreeRTOS_write( Network*, unsigned char*, int, int );

/**
 * Closes the socket on a non-TLS enabled network
 *
 * \param[in]     n		     Network pointer
 */
void FreeRTOS_disconnect( Network*);

/**
 * Initialize Network object
 *
 * \param[in/out] n		     Network pointer to configure
 *
 * \returns       returns configured network
 */
void NetworkInit( Network* n);

/**
 * Default MQTT network connection without TLS
 *
 * \param[in]     n		     Configured Network pointer
 * \param[in]     addr       Hostname to connect to
 * \param[in]     port    	 Port to connect to
 *
 * \returns       returns negative value on failure
 */
int NetworkConnect( Network*, char*, int );

/**
 * Default MQTT network connection without TLS
 *
 * \param[in]     n		     Configured Network pointer
 * \param[in]     addr       IP address to connect to
 * \param[in]     port    	 Port to connect to
 *
 * \returns       returns negative value on failure
 */
int NetworkConnectIP( Network*, uint32_t, int );


#endif
