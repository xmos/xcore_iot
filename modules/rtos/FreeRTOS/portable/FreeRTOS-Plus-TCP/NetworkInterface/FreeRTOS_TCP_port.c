// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_TCP_port.h"

/* XMOS includes */
#include "random.h"


static random_generator_t rng;

BaseType_t xInit_RNG( void )
{
BaseType_t xReturn;
    if( rng == (random_generator_t)NULL )
    {
        rng = random_create_generator_from_seed( TCPPORT_INITIAL_RANDOM_SEED );
    }

    if( rng != (random_generator_t)NULL )
    {
        xReturn = pdPASS;
    }
    else
    {
        xReturn = pdFAIL;
    }
    return xReturn;
}

UBaseType_t uxRand( void )
{
UBaseType_t uxRetVal;
    uxRetVal = random_get_random_number( &rng );
    return uxRetVal;
}

#if !(__iot_secure_sockets_h_exists__)
BaseType_t xApplicationGetRandomNumber( uint32_t *pulNumber )
{
BaseType_t xRetVal;
UBaseType_t uxVal;

    xRetVal = xInit_RNG();
    uxVal = uxRand();
    *(pulNumber) = uxVal;

    return xRetVal;
}

/* Generate ISN per RFC 6528
 * ISN = M + F(localip, localport, remoteip, remoteport, secretkey)
 *       where M is a 4 us timer, F is a pseudorandom function */
#warning ulApplicationGetNextSequenceNumber needs to be modified for real usage
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    // TODO: when cryptolibrary is integrated, replace with md5 hash
    uint32_t ulISN;
    // TEMPORARY
    ulISN = random_get_random_number( &rng );
    // TEMPORARY END

    // get 4us timer value
    // create ctx
    // hash start
    // hash update ulSourceAddress
    // hash update usSourcePort
    // hash update ulDestinationAddress
    // hash update usDestinationPort
    // hash update secret
    // hash finish
    return ulISN;
}
#endif

