// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"

/* Library headers */

/* App headers */
#include "network.h"
#include "app_conf.h"

const uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_0,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_1,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_2,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_3 };

const uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_0,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_1,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_2,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_3 };

const uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_0,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_1,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_2,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_3 };

const uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_0,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_1,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_2,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_3 };

const uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_0,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_1,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_2,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_3,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_4,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_5 };

void initalize_FreeRTOS_IP( void )
{
    FreeRTOS_IPInit(
        ucIPAddress,
        ucNetMask,
        ucGatewayAddress,
        ucDNSServerAddress,
        ucMACAddress );
}

