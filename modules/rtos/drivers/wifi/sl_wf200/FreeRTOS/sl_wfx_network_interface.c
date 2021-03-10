/*
FreeRTOS+TCP V2.0.11
Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 http://aws.amazon.com/freertos
 http://www.FreeRTOS.org
*/


/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_TCP_port.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "sl_wfx.h"
#include "sl_wfx_host.h"

/********************************************
 * These options are no longer supported. The code that used to
 * support these options is being left in place for the future if
 * support for these is ever added back in.
 ********************************************/
#if ipconfigZERO_COPY_RX_DRIVER != 0
#error The WF200 WiFi FreeRTOS driver does not support the ipconfigZERO_COPY_RX_DRIVER option
#endif

#if ipconfigZERO_COPY_TX_DRIVER != 0
#error The WF200 WiFi FreeRTOS driver does not support the ipconfigZERO_COPY_TX_DRIVER option
#endif
/********************************************/


/* FreeRTOS_IP_Private.h mistakenly only declares this when
 * ipconfigZERO_COPY_TX_DRIVER is not 0. */
#if ipconfigZERO_COPY_RX_DRIVER != 0
NetworkBufferDescriptor_t *pxPacketBuffer_to_NetworkBuffer( const void *pvBuffer );
#endif

/*
 * FreeRTOSIPConfig.h should include the header that provides the
 * declarations for WIFI_GetLock() and WIFI_ReleaseLock().
 * When using Amazon FreeRTOS this should be "iot_wifi.h".
 */

#define PRINT_MAC_ADDR( A ) rtos_printf("%x:%x:%x:%x:%x:%x\n", A[0], A[1], A[2], A[3], A[4], A[5])

void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t *rx_buffer)
{
uint8_t *frame_buffer;
NetworkBufferDescriptor_t *pxNetworkBuffer = NULL;
int frame_length;

    frame_buffer = &rx_buffer->body.frame[ rx_buffer->body.frame_padding ];
    frame_length = rx_buffer->body.frame_length;

    /* Allocate a new network buffer */
    if ( frame_length > 0 )
    {
#if ipconfigZERO_COPY_RX_DRIVER == 0
        pxNetworkBuffer = pxGetNetworkBufferWithDescriptor( frame_length, 0 );

        if( pxNetworkBuffer != NULL )
#else
        pxNetworkBuffer = pxPacketBuffer_to_NetworkBuffer( &rx_buffer->body.frame[ SL_WFX_NORMAL_FRAME_PAD_LENGTH ] );
        xassert( pxNetworkBuffer->pucEthernetBuffer == &rx_buffer->body.frame[ SL_WFX_NORMAL_FRAME_PAD_LENGTH ] );

        pxNetworkBuffer->xDataLength = frame_length;

        if ( rx_buffer->body.frame_padding != SL_WFX_NORMAL_FRAME_PAD_LENGTH )
        {
            memmove( pxNetworkBuffer->pucEthernetBuffer, frame_buffer, frame_length );
            //rtos_printf("moving %d bytes (padding is %d)\n", frame_length, rx_buffer->body.frame_padding);
        }
        else
        {
            //rtos_printf("leaving %d bytes where it is\n", frame_length);
        }
#endif
        {
            IPStackEvent_t xRxEvent = { eNetworkRxEvent, NULL };

#if ipconfigZERO_COPY_RX_DRIVER == 0
            memcpy( pxNetworkBuffer->pucEthernetBuffer, frame_buffer, frame_length );
#endif
            xRxEvent.pvData = ( void * ) pxNetworkBuffer;

            /* Data was received and stored.  Send a message to the IP
            task to let it know. */
            if( xSendEventStructToIPTask( &xRxEvent, ( TickType_t ) 0 ) == pdFAIL )
            {
                vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
                iptraceETHERNET_RX_EVENT_LOST();
                rtos_printf("eth data lost 1\n", frame_length);
            }
            else
            {
                iptraceNETWORK_INTERFACE_RECEIVE();
            }
        }
#if ipconfigZERO_COPY_RX_DRIVER == 0
        else
        {
            /* There is not a new network buffer available */

            iptraceETHERNET_RX_EVENT_LOST();
            rtos_printf("eth data lost 2\n", frame_length);
        }
#endif
    }
}

BaseType_t xNetworkInterfaceInitialise( void )
{
BaseType_t xStatus;
static uint8_t ucOriginalFreeRTOSMACAddress[ ipMAC_ADDRESS_LENGTH_BYTES ];
static int xOriginalFreeRTOSMACAddressSet;
WIFIDeviceMode_t mode;

    xStatus = xInit_RNG();
    xassert( xStatus != pdFAIL );
#ifndef SL_WFX_USE_SECURE_LINK
    xassert( ipBUFFER_PADDING >= sizeof( void * ) + sizeof( sl_wfx_send_frame_req_t ) );
    xassert( sizeof( sl_wfx_received_ind_t ) <= sizeof( sl_wfx_send_frame_req_t ) );
#endif

    if( xOriginalFreeRTOSMACAddressSet == pdFALSE )
    {
        memcpy( ucOriginalFreeRTOSMACAddress, FreeRTOS_GetMACAddress(), ipMAC_ADDRESS_LENGTH_BYTES );
        xOriginalFreeRTOSMACAddressSet = pdTRUE;
    }

    if( xGetPhyLinkStatus() == pdFAIL )
    {
        EventBits_t bits;

        /* WiFi is not yet connected to an AP */

        while( sl_wfx_context == NULL || sl_wfx_event_group == NULL)
        {
            /* sl_wfx_init() has not yet set the context or created the
            event group. Wait a bit and check again. */
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        /* The WFX200 has not yet completed initialization.
        Wait for it to do so. */
        do {
            bits = xEventGroupWaitBits(sl_wfx_event_group,
                                       SL_WFX_INITIALIZED,
                                       pdFALSE, /* Do not clear this bit */
                                       pdTRUE,
                                       portMAX_DELAY);
        } while( ( bits & SL_WFX_INITIALIZED ) == 0 );

        if( WIFI_GetLock() == eWiFiSuccess )
        {
            if( ( sl_wfx_context->state & SL_WFX_STARTED ) == 0 )
            {
                /* This probably shouldn't happen */
                WIFI_ReleaseLock();
                return pdFAIL;
            }

            if( WIFI_IsConnected() == pdFALSE )
            {
                BaseType_t xMAC_addr_init;
                sl_wfx_mac_address_t unsetMAC = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

                /* The WFX200 has been initialized but is not connected to an AP.
                This is a good time to set the MAC addresses in the WFX200 if
                necessary. */

                xMAC_addr_init = memcmp( ucOriginalFreeRTOSMACAddress,
                                         unsetMAC.octet,
                                         ( size_t ) ipMAC_ADDRESS_LENGTH_BYTES ) != 0;

                if( xMAC_addr_init == pdTRUE )
                {
                    /* A non-zero MAC address was passed to FreeRTOS_IPInit(). Therefore
                    we will set the MAC addresses for both interfaces in the WFX200 to it. */

                    /* Set hardware MAC with the one passed to FreeRTOS_IPInit() */
                    sl_wfx_set_mac_address( ( sl_wfx_mac_address_t * ) ucOriginalFreeRTOSMACAddress, SL_WFX_STA_INTERFACE );
                    sl_wfx_set_mac_address( ( sl_wfx_mac_address_t * ) ucOriginalFreeRTOSMACAddress, SL_WFX_SOFTAP_INTERFACE );
                    memcpy( sl_wfx_context->mac_addr_0.octet, ucOriginalFreeRTOSMACAddress, ipMAC_ADDRESS_LENGTH_BYTES );
                    memcpy( sl_wfx_context->mac_addr_1.octet, ucOriginalFreeRTOSMACAddress, ipMAC_ADDRESS_LENGTH_BYTES );
                }

                rtos_printf("WF200 station MAC address set to ");
                PRINT_MAC_ADDR(sl_wfx_context->mac_addr_0.octet);
                rtos_printf("WF200 SoftAP MAC address set to ");
                PRINT_MAC_ADDR(sl_wfx_context->mac_addr_1.octet);

                WIFI_ReleaseLock();

                do {
                    bits = xEventGroupWaitBits(sl_wfx_event_group,
                                               SL_WFX_CONNECT | SL_WFX_START_AP,
                                               pdFALSE, /* Do not clear these bits */
                                               pdFALSE,
                                               portMAX_DELAY);
                } while( ( bits & ( SL_WFX_CONNECT | SL_WFX_START_AP ) ) == 0 );
            }
            else
            {
                /* The WIFI managed to connect to an AP before we got a chance to
                check to see if the MAC address should get changed. */
                WIFI_ReleaseLock();
            }
        }
        else
        {
            /* Failed to get the WIFI lock */
            return pdFAIL;
        }
    }

    /* The WIFI is connected to an AP. Ensure that the FreeRTOS
    MAC addr is the same as the one used by the wfx200. */
    WIFI_GetMode( &mode );

    if( mode == eWiFiModeStation )
    {
        FreeRTOS_UpdateMACAddress( sl_wfx_context->mac_addr_0.octet );
    }
    else
    {
        FreeRTOS_UpdateMACAddress( sl_wfx_context->mac_addr_1.octet );
    }

    rtos_printf("FreeRTOS+TCP MAC address set to ");
    PRINT_MAC_ADDR(FreeRTOS_GetMACAddress());

    /* Wait a moment to give things some time to settle. On some networks it
    seems that the initial DHCP discover is lost when sent out too soon
    after connecting to the AP. */
    vTaskDelay( pdMS_TO_TICKS( 250 ) );

    return pdPASS;
}

BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend )
{
sl_wfx_send_frame_req_t *tx_buffer;
sl_status_t result;
uint32_t frame_length;
WIFIDeviceMode_t mode;

    /* Obtaining the WiFi lock and then only sending if the WiFi is connected
    ensures that sends only ever happen when there is a WiFi connection, and
    that the application will not be able to disconnect before the send is
    completed. */
    if( WIFI_GetLock() == eWiFiSuccess )
    {
        if( WIFI_IsConnected() != pdFALSE )
        {

            /* When not using the secure link, the sl_wfx_send_frame_req_t header is
             * inserted immediately before the beginning of the Ethernet frame, inside
             * the frame buffer already allocated by FreeRTOS+TCP. The same cannot be done
             * when using the secure link (at least not with BufferAllocation_2) because
             * it also requires a tag following the frame which isn't possible to allocate
             * space for without modifying FreeRTOS+TCP. It might be possible with
             * BufferAllocation_1.
             *
             * The space for the sl_wfx_send_frame_req_t header is made by requiring that
             * ipconfigPACKET_FILLER_SIZE be at least sizeof(sl_wfx_send_frame_req_t).
             * This creates space between the beginning of the buffer and the beginning
             * of the Ethernet Frame, the first sizeof(void *) bytes of which are used to
             * store a pointer back to the NetworkBufferDescriptor_t struct. */

#ifndef SL_WFX_USE_SECURE_LINK
            frame_length = pxNetworkBuffer->xDataLength; /* is the round up really necessary? */
            tx_buffer = ( sl_wfx_send_frame_req_t * ) ( pxNetworkBuffer->pucEthernetBuffer - sizeof( sl_wfx_send_frame_req_t ) );
#else
            frame_length = SL_WFX_ROUND_UP(pxNetworkBuffer->xDataLength, 2);
            result = sl_wfx_allocate_command_buffer((sl_wfx_generic_message_t**)(&tx_buffer),
                                                    SL_WFX_SEND_FRAME_REQ_ID,
                                                    SL_WFX_TX_FRAME_BUFFER,
                                                    frame_length + sizeof(sl_wfx_send_frame_req_t));

            if( result == SL_STATUS_OK )
            {
                memcpy( tx_buffer->body.packet_data, pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength );
#endif

                /* Send the packet */
                WIFI_GetMode( &mode );
                result = sl_wfx_send_ethernet_frame(tx_buffer,
                                                    frame_length,
                                                    mode == eWiFiModeStation ? SL_WFX_STA_INTERFACE : SL_WFX_SOFTAP_INTERFACE,
                                                    0);
#ifdef SL_WFX_USE_SECURE_LINK
                sl_wfx_free_command_buffer((sl_wfx_generic_message_t*) tx_buffer,
                                           SL_WFX_SEND_FRAME_REQ_ID,
                                           SL_WFX_TX_FRAME_BUFFER);
#endif
                if( result == SL_STATUS_OK )
                {
                    /* Call the standard trace macro to log the send event. */
                    iptraceNETWORK_INTERFACE_TRANSMIT();
                }
                else
                {
                    iptraceSTACK_TX_EVENT_LOST();
                }
#ifdef SL_WFX_USE_SECURE_LINK
            }
            else
            {
                iptraceNETWORK_INTERFACE_TRANSMIT();
            }
#endif
        }

        WIFI_ReleaseLock();
    }

#if ipconfigZERO_COPY_TX_DRIVER == 0
    if( xReleaseAfterSend != pdFALSE )
    {
        vReleaseNetworkBufferAndDescriptor( pxNetworkBuffer );
    }
#else
	xassert(xReleaseAfterSend);
	/*
	 * The DMA ISR will free the buffer
	 */
#endif

    return pdTRUE;
}

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    /* Only required if using BufferAllocation_1.c, which uses
     * preallocated static network buffers */
}

BaseType_t xGetPhyLinkStatus( void )
{
    return WIFI_IsConnected();
}
