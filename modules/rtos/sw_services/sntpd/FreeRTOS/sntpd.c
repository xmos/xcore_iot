// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "sntpd.h"

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
/**
 * NTP Timestamp format as defined in RFC5905
 *
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                            Seconds                            |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                            Fraction                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct {
  uint32_t seconds;		/* Seconds passed since Jan 1, 1990 */
  uint32_t fraction;	/* Fractional field */
} sntp_timestamp_t;

/**
 * NTP packet format as defined in RFC4330
 *
 *                         1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                          Root  Delay                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       Root  Dispersion                         |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                     Reference Identifier                       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                                |
 *    |                    Reference Timestamp (64)                    |
 *    |                                                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                                |
 *    |                    Originate Timestamp (64)                    |
 *    |                                                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                                |
 *    |                     Receive Timestamp (64)                     |
 *    |                                                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                                |
 *    |                     Transmit Timestamp (64)                    |
 *    |                                                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                 Key Identifier (optional) (32)                 |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                                                                |
 *    |                                                                |
 *    |                 Message Digest (optional) (128)                |
 *    |                                                                |
 *    |                                                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct __attribute__ ((__packed__)) {
  uint8_t flags; 							/* Flags: 0:1 Leap indicator, 2:4 Version Number,
   	   	   	   	   	   	   	   	   	   	   	   5:7 Mode */
  uint8_t stratum; 							/* Stratum */
  uint8_t poll;  							/* Maximum successive message interval in log2
  	  	  	  	  	  	  	  	  	  	  	   seconds */
  int8_t precision; 						/* Precision of system clock in log2 seconds */
  int32_t rootDelay; 						/* Total round trip delay to primary reference */
  uint32_t rootDispersion; 					/* Maximum error due to clock freq tolerance */
  int8_t referenceID[4]; 					/* Reference source identifier */
  sntp_timestamp_t referenceTimestamp; 		/* Time the system clock was last set or corrected */
  sntp_timestamp_t originateTimestamp; 		/* Time the request departed the client for the
  	  	  	  	  	  	  	  	  	  	  	   server */
  sntp_timestamp_t receiveTimestamp; 		/* Time the request arrived at the server or reply
  	  	  	  	  	  	  	  	  	  	  	   arrived at client */
  sntp_timestamp_t transmitTimestamp; 		/* Time the request departed the client or the
   	   	   	   	   	   	   	   	   	   	   	   reply departed the server */
} sntp_packet_t;

static int time_synced = pdFALSE;

int is_time_synced( void )
{
	return time_synced;
}

static void sntpd_task( void *args )
{
	int failed_attempts = 0;
	uint32_t ip = 0;
	struct freertos_sockaddr ntp_addr;
	uint32_t addr_len = sizeof( ntp_addr );
	sntp_packet_t packet;
	Socket_t sntp_socket = NULL;

	while( 1 )
	{
		while( FreeRTOS_IsNetworkUp() != pdTRUE )
		{
			vTaskDelay( pdMS_TO_TICKS(100) );
		}

#if( SNTPD_USER_TIME_SERVER == 0 )
		/* Search default list, use first server resolved */
		for( int i=0; i<(sizeof( default_time_servers ) / sizeof( default_time_servers[0] )); i++ )
		{
			ip = FreeRTOS_gethostbyname( default_time_servers[i] );
#else
		for( int i=0; i<(sizeof( user_time_servers ) / sizeof( user_time_servers[0] )); i++ )
		{
			ip = FreeRTOS_gethostbyname( user_time_servers[i] );
#endif
//			char buf[16];
//			FreeRTOS_inet_ntoa( ip, buf );
//			rtos_printf( "%s is %s\n", default_time_servers[i], buf );

			if( ip > 0)
			{
				break;
			}
		}

		ntp_addr.sin_addr = ip;
		ntp_addr.sin_port = FreeRTOS_htons( SNTPD_PORT );

		packet.flags = SNTPD_FLAGS_LI_NOT_SYNCHRONIZED | SNTPD_FLAGS_VN_4 | SNTPD_FLAGS_MODE_CLIENT;
		packet.poll = 0;						/* Server significant only */
		packet.precision = 0x00;				/* Server significant only */
		packet.rootDelay = 0x0000;				/* Server significant only */
		packet.rootDispersion = 0x00000000;		/* Server significant only */

		sntp_socket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );

		if( sntp_socket != FREERTOS_INVALID_SOCKET )
		{
			BaseType_t xReceiveTimeOut = pdMS_TO_TICKS( SNTPD_RX_TIMEOUT_MS );

			FreeRTOS_bind( sntp_socket, &ntp_addr, sizeof( ntp_addr ) );
			FreeRTOS_setsockopt( sntp_socket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );

			while( 1 )
			{
//				rtos_printf("send ntp packet.\n");

				rtos_time_t cur = rtos_time_get();

				packet.referenceTimestamp.seconds = FreeRTOS_htonl( cur.seconds + EPOCH );
				packet.referenceTimestamp.fraction = FreeRTOS_htonl( 0 );	/* fractional resolution not needed for now */

				packet.transmitTimestamp.seconds = FreeRTOS_htonl( cur.seconds + EPOCH );
				packet.transmitTimestamp.fraction = FreeRTOS_htonl( 0 );	/* fractional resolution not needed for now */

				FreeRTOS_sendto( sntp_socket, &packet, sizeof( sntp_packet_t ), 0, &ntp_addr, addr_len );

				BaseType_t rx_len = 0;
				sntp_packet_t* rxpacket = NULL;
				rx_len = FreeRTOS_recvfrom( sntp_socket,
											&rxpacket,
											0,
											FREERTOS_ZERO_COPY,
											&ntp_addr,
											&addr_len );

				if( rx_len < 0 )
				{
					failed_attempts++;
	//				rtos_printf("sntpd packet ret:%d\n", rx_len);
				}
				else
				{
					if( rx_len < sizeof( rxpacket ) )
					{
						failed_attempts++;
	//					rtos_printf("sntpd bad response to ntp packet.\n");
					}
					else
					{
	//					rtos_printf("got response from ntp server %d\n", rx_len);

						rtos_time_t now;
						now.seconds = FreeRTOS_htonl( rxpacket->receiveTimestamp.seconds ) - EPOCH;
						now.microseconds = FreeRTOS_htonl( rxpacket->receiveTimestamp.fraction ) / UINT_MAX;
						rtos_time_set( now );

						time_synced = pdTRUE;

						struct tm *info;
						info = gmtime( (time_t * )(&now.seconds) );
						rtos_printf( "NTP time: %d/%d/%02d %2d:%02d:%02d\n",
									 (int)info->tm_mday,
									 (int)info->tm_mon + 1,
									 (int)info->tm_year + 1900,
									 (int)info->tm_hour,
									 (int)info->tm_min,
									 (int)info->tm_sec) ;

						if( ( ( uint8_t ) rxpacket->stratum ) == SNTPD_STRATUM_KISS_OF_DEATH )
						{
							failed_attempts = SNTPD_RESET_AFTER_X_FAILURES; /* for a reset */
						}
					}

					FreeRTOS_ReleaseUDPPayloadBuffer( ( void * )rxpacket );
				}

				if( failed_attempts >= SNTPD_RESET_AFTER_X_FAILURES )
				{
				    FreeRTOS_closesocket( sntp_socket );
				    failed_attempts = 0;
					sntp_socket = NULL;
					break;
				}
				vTaskDelay( pdMS_TO_TICKS( SNTPD_POLL_RATE_MS ) );
			}
		}
	}
}

void sntp_create( UBaseType_t priority )
{
    xTaskCreate( sntpd_task, SNTPD_TASK_NAME, SNTPD_TASK_STACKSIZE, NULL, priority, NULL );
}


