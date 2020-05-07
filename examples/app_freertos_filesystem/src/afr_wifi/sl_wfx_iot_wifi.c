/*
 * sl_wfx_iot_wifi.c
 *
 *  Created on: Jan 21, 2020
 *      Author: mbruno
 */

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_DHCP.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"
#include "brd8023a_pds.h"

/* Application headers */
#include "sl_wfx_iot_wifi.h"
#include "dhcpd.h"

#define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define HWADDR_ARG(hwaddr) hwaddr.ucBytes[0], hwaddr.ucBytes[1], hwaddr.ucBytes[2], hwaddr.ucBytes[3], hwaddr.ucBytes[4], hwaddr.ucBytes[5]

#define SL_WFX_CONNECT_TIMEOUT_MS 2000
#define SL_WFX_DISCONNECT_TIMEOUT_MS 500

static sl_wfx_context_t wfx_ctx;

static WIFIDeviceMode_t wifi_mode = eWiFiModeStation;

static struct
{
    int configured;
    sl_wfx_security_mode_t security;                      /**< Wi-Fi Security. */
    uint32_t ssid_length;                                 /**< SSID length not including NULL termination. */
    uint16_t password_length;                             /**< Password length not including null termination. */
    uint16_t channel;                                     /**< Channel number. */
    uint8_t ssid[ wificonfigMAX_SSID_LEN + 1 ];           /**< SSID of the Wi-Fi network to join with a NULL termination. */
    uint8_t password[ wificonfigMAX_PASSPHRASE_LEN + 1 ]; /**< Password needed to join the AP with a NULL termination. */
} wifi_ap_settings;


static SemaphoreHandle_t wifi_lock;
static QueueHandle_t ping_reply_queue;

WIFIReturnCode_t WIFI_GetLock( void )
{
    WIFIReturnCode_t ret;

    if( wifi_lock != NULL )
    {
        if( xSemaphoreTakeRecursive( wifi_lock, pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS ) ) == pdPASS )
        {
            EventBits_t bits;
            bits = xEventGroupGetBits( sl_wfx_event_group );
            if( ( bits & SL_WFX_INITIALIZED ) != 0 )
            {
                ret = eWiFiSuccess;
            }
            else
            {
                xSemaphoreGiveRecursive( wifi_lock );
                ret = eWiFiFailure;
            }

            ret = eWiFiSuccess;
        }
        else
        {
            ret = eWiFiTimeout;
        }
    }
    else
    {
        ret = eWiFiFailure;
    }

    return ret;
}

void WIFI_ReleaseLock( void )
{
    if( wifi_lock != NULL )
    {
        xSemaphoreGiveRecursive( wifi_lock );
    }
}

WIFIReturnCode_t WIFI_On( void )
{
    WIFIReturnCode_t ret = eWiFiFailure;
    sl_status_t sl_ret;

    if( sl_wfx_context != NULL && sl_wfx_context->state & SL_WFX_STARTED )
    {
        ret = eWiFiSuccess;
    }

    if( ret != eWiFiSuccess )
    {
#if XCORE200_MAB
        sl_wfx_host_set_hif( bitstream_spi_devices[ BITSTREAM_SPI_DEVICE_A ],
                             bitstream_gpio_devices[ BITSTREAM_GPIO_DEVICE_A ],
                             gpio_1I, 0,  /* header pin 9 */
                             gpio_1P, 0,  /* header pin 10 */
                             gpio_1J, 0 ); /* header pin 12 */
#elif XCOREAI_EXPLORER
        sl_wfx_host_set_hif( bitstream_spi_devices[ BITSTREAM_SPI_DEVICE_A ],
                             bitstream_gpio_devices[ BITSTREAM_GPIO_DEVICE_A ],
                             gpio_1I, 0,  /* IRQ */
                             gpio_4E, 0,  /* WUP */
                             gpio_4E, 1 ); /* RST */
#endif

        sl_wfx_host_set_pds( pds_table_brd8023a, SL_WFX_ARRAY_COUNT( pds_table_brd8023a ) );

        sl_ret = sl_wfx_init( &wfx_ctx );

        if( sl_ret == SL_STATUS_OK )
        {
            if( wifi_lock == NULL )
            {
                wifi_lock = xSemaphoreCreateRecursiveMutex();
                configASSERT( wifi_lock != NULL );
            }
            if( ping_reply_queue == NULL )
            {
                ping_reply_queue = xQueueCreate(1, sizeof(uint16_t));
                configASSERT( ping_reply_queue != NULL);
            }

            wifi_mode = eWiFiModeStation;
            xEventGroupClearBits( sl_wfx_event_group, 0x00FFFFFF );
            xEventGroupSetBits( sl_wfx_event_group, SL_WFX_INITIALIZED );
            ret = eWiFiSuccess;
        }
    }

    return ret;
}

/*
 * TODO: Verify that the host code that
 * sl_wfx_shutdown() calls does the right
 * things.
 */
WIFIReturnCode_t WIFI_Off( void )
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        sl_status_t sl_ret;

        WIFI_StopAP();
        WIFI_Disconnect();

        /* Ensure these bits are cleared in case either of the above
        calls failed. They might if the wifi chip is in a bad state. */
        sl_wfx_context->state &= ~( SL_WFX_AP_INTERFACE_UP | SL_WFX_STA_INTERFACE_CONNECTED );
        xEventGroupClearBits( sl_wfx_event_group, SL_WFX_START_AP | SL_WFX_CONNECT | SL_WFX_INITIALIZED );

        sl_ret = sl_wfx_deinit();
        if( sl_ret != SL_STATUS_OK )
        {
            ret = eWiFiFailure;
        }

        WIFI_ReleaseLock();
    }
    else if( sl_wfx_context == NULL || ( sl_wfx_context->state & SL_WFX_STARTED ) == 0 )
    {
        /* If the lock cannot be obtained it might be because the
        wifi chip is aleady off. If it is then just return success. */
        ret = eWiFiSuccess;
    }

    return ret;
}

void sl_wfx_disconnect_callback(uint8_t *mac, sl_wfx_reason_t reason)
{
    ( void ) mac;
    sl_wfx_host_log( "Disconnected %d\n", reason );
    sl_wfx_context->state &= ~SL_WFX_STA_INTERFACE_CONNECTED;
    xEventGroupClearBits( sl_wfx_event_group, SL_WFX_CONNECT );
    xEventGroupSetBits( sl_wfx_event_group, SL_WFX_DISCONNECT );
    FreeRTOS_NetworkDown();
}

WIFIReturnCode_t WIFI_Disconnect( void )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( sl_wfx_context->state & SL_WFX_STA_INTERFACE_CONNECTED )
        {
            sl_status_t sl_ret;

            sl_ret = sl_wfx_send_disconnect_command();

            if( sl_ret == SL_STATUS_OK )
            {
                bits = xEventGroupWaitBits( sl_wfx_event_group,
                                            SL_WFX_DISCONNECT,
                                            pdTRUE,
                                            pdTRUE,
                                            pdMS_TO_TICKS( SL_WFX_DISCONNECT_TIMEOUT_MS ) );

                if( ( bits & SL_WFX_DISCONNECT ) == 0 )
                {
                    ret = eWiFiFailure;
                }
            }
            else
            {
                ret = eWiFiFailure;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

void sl_wfx_connect_callback( uint8_t *mac, sl_wfx_fmac_status_t status )
{
    int connected = 0;

    ( void ) mac;

    switch ( status )
    {
    case WFM_STATUS_SUCCESS:
        sl_wfx_host_log( "Connection succeeded\n" );
        connected = 1;
        break;

    case WFM_STATUS_NO_MATCHING_AP:
        sl_wfx_host_log( "Connection failed, access point not found\n" );
        break;

    case WFM_STATUS_CONNECTION_ABORTED:
        sl_wfx_host_log( "Connection aborted\n" );
        break;

    case WFM_STATUS_CONNECTION_TIMEOUT:
        sl_wfx_host_log( "Connection timeout\n" );
        break;

    case WFM_STATUS_CONNECTION_REJECTED_BY_AP:
        sl_wfx_host_log( "Connection rejected by the access point\n" );
        break;

    case WFM_STATUS_CONNECTION_AUTH_FAILURE:
        sl_wfx_host_log( "Connection authentication failure\n" );
        break;

    default:
        sl_wfx_host_log( "Connection attempt error\n" );
        break;
    }

    if( connected )
    {
        sl_wfx_context->state |= SL_WFX_STA_INTERFACE_CONNECTED;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_CONNECT );
    }
    else
    {
        sl_wfx_context->state &= ~SL_WFX_STA_INTERFACE_CONNECTED;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_CONNECT_FAIL );
    }
}

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    sl_wfx_security_mode_t security;
    EventBits_t bits;
    WIFIReturnCode_t ret;

    configASSERT( pxNetworkParams != NULL );

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        switch ( pxNetworkParams->xSecurity )
        {
        case eWiFiSecurityOpen:
            security = WFM_SECURITY_MODE_OPEN;
            break;
        case eWiFiSecurityWEP:
            security = WFM_SECURITY_MODE_WEP;
            break;
        case eWiFiSecurityWPA:
            security = WFM_SECURITY_MODE_WPA2_WPA1_PSK;
            break;
        case eWiFiSecurityWPA2:
            security = WFM_SECURITY_MODE_WPA2_PSK;
            break;
        default:
            ret = eWiFiFailure;
            break;
        }

        if( ret == eWiFiSuccess )
        {
            ret = WIFI_SetMode( eWiFiModeStation );
        }

        if( ret == eWiFiSuccess )
        {
            ret = WIFI_Disconnect();
        }

        if( ret == eWiFiSuccess )
        {
            sl_status_t sl_ret;

            sl_wfx_host_log( "Connect to: %s(%d):%s(%d) on ch %d with security %d\n",
                             pxNetworkParams->pcSSID,
                             pxNetworkParams->ucSSIDLength,
                             pxNetworkParams->pcPassword,
                             pxNetworkParams->ucPasswordLength,
                             pxNetworkParams->cChannel,
                             security );

            /*
             * Ensure the connect fail bit is cleared as it is not automatically
             * cleared below. The connect bit should be cleared at this point.
             */
            bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_CONNECT_FAIL );
            configASSERT( ( bits & SL_WFX_CONNECT ) == 0 );

            sl_ret = sl_wfx_send_join_command( ( const uint8_t * ) pxNetworkParams->pcSSID,
                                               pxNetworkParams->ucSSIDLength,
                                               NULL,
                                               pxNetworkParams->cChannel,
                                               security,
                                               0,
                                               1,
                                               ( const uint8_t * ) pxNetworkParams->pcPassword,
                                               pxNetworkParams->ucPasswordLength,
                                               NULL,
                                               0);

            if( sl_ret == SL_STATUS_OK )
            {
                bits = xEventGroupWaitBits( sl_wfx_event_group,
                                            SL_WFX_CONNECT | SL_WFX_CONNECT_FAIL,
                                            pdFALSE, /* Do not clear these bits */
                                            pdFALSE,
                                            pdMS_TO_TICKS( SL_WFX_CONNECT_TIMEOUT_MS ) );

                if( ( bits & SL_WFX_CONNECT ) == 0 )
                {
                    ret = eWiFiFailure;
                }
            }
            else
            {
                ret = eWiFiFailure;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_Reset( void )
{
    WIFIReturnCode_t ret;

    ret = WIFI_Off();

    if( ret == eWiFiSuccess )
    {
        ret = WIFI_On();
    }
    else
    {
        rtos_printf("WIFI_Off() returned %d\n", ret);
    }

    return ret;
}

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( wifi_mode != xDeviceMode )
        {
            if( xDeviceMode == eWiFiModeStation )
            {
                ret = WIFI_StopAP();
            }
            else if( xDeviceMode == eWiFiModeAP )
            {
                ret = WIFI_Disconnect();
            }
            else
            {
                /* Does not support P2P */
                ret = eWiFiFailure;
            }

            if( ret == eWiFiSuccess )
            {
                wifi_mode = xDeviceMode;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t *pxDeviceMode )
{
    WIFIReturnCode_t ret;

    configASSERT( pxDeviceMode != NULL );

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        *pxDeviceMode = wifi_mode;
        WIFI_ReleaseLock();
    }

    return ret;
}

/*
 * TODO: later?
 */
WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t *pusIndex )
{
    configASSERT( pxNetworkProfile != NULL );
    configASSERT( pusIndex != NULL );
    return eWiFiNotSupported;
}

/*
 * TODO: later?
 */
WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t *pxNetworkProfile,
                                  uint16_t usIndex )
{
    configASSERT( pxNetworkProfile != NULL );
    return eWiFiNotSupported;
}

/*
 * TODO: later?
 */
WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    return eWiFiNotSupported;
}

void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier)
{
    if (eStatus == eSuccess) {
        dhcpcd_ping_reply_received( usIdentifier );
        xQueueOverwrite(ping_reply_queue, &usIdentifier);
    }
}

WIFIReturnCode_t WIFI_Ping( uint8_t *pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    uint32_t ip;
    uint32_t arp_ip;
    MACAddress_t arp_mac;
    TickType_t last_wake;
    int i;
    int good = 0;

    configASSERT( pucIPAddr != NULL );

    memcpy( &ip, pucIPAddr, sizeof( uint32_t ) );
    arp_ip = ip;

    last_wake = xTaskGetTickCount();

    if( eARPGetCacheEntry( &arp_ip, &arp_mac ) != eARPCacheHit )
    {
        char ip_str[16];
        FreeRTOS_inet_ntoa(ip, ip_str);
        rtos_printf("%s is unknown, will ARP before pinging\n", ip_str);

        FreeRTOS_OutputARPRequest( arp_ip );
        vTaskDelayUntil( &last_wake, pdMS_TO_TICKS( ulIntervalMS ) );
    }

    for( i = 0; i < usCount; i++ )
    {
        BaseType_t ret = pdFAIL;
        uint16_t ping_number_out;
        ping_number_out = FreeRTOS_SendPingRequest( ip, 48, pdMS_TO_TICKS( ulIntervalMS ) );
        if( ping_number_out != pdFAIL )
        {
            uint16_t ping_number_in;
            do
            {
                ret = xQueueReceive( ping_reply_queue, &ping_number_in, pdMS_TO_TICKS( ulIntervalMS ) );
            }
            while( ret == pdPASS && ping_number_out != ping_number_in );
        }
        if( ret == pdPASS )
        {
            rtos_printf( "Received ping reply %d\n", ping_number_out );
            good++;
        }
        else
        {
            rtos_printf( "Timed out waiting for ping reply %d\n", ping_number_out );
        }

        vTaskDelayUntil( &last_wake, pdMS_TO_TICKS( ulIntervalMS ) );
    }

    rtos_printf( "Received %d/%d replies\n", good, usCount );

    return good > 0 ? eWiFiSuccess : eWiFiFailure;
}

WIFIReturnCode_t WIFI_GetIP( uint8_t *pucIPAddr )
{
    WIFIReturnCode_t ret = eWiFiFailure;
    uint32_t ip;
    configASSERT( pucIPAddr != NULL );

    ip = FreeRTOS_GetIPAddress();

    if( ip != 0 ) {
        memcpy( pucIPAddr, &ip, sizeof( uint32_t ) );
        ret = eWiFiSuccess;
    }
    return ret;
}

WIFIReturnCode_t WIFI_GetMAC( uint8_t *pucMac )
{
    WIFIReturnCode_t ret;

    configASSERT( pucMac != NULL );

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( wifi_mode == eWiFiModeStation )
        {
            memcpy( pucMac, sl_wfx_context->mac_addr_0.octet, wificonfigMAX_BSSID_LEN );
        }
        else if( wifi_mode == eWiFiModeAP )
        {
            memcpy( pucMac, sl_wfx_context->mac_addr_1.octet, wificonfigMAX_BSSID_LEN );
        }
        else
        {
            ret = eWiFiFailure;
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_GetHostIP( char *pcHost,
                                 uint8_t *pucIPAddr )
{
    WIFIReturnCode_t ret = eWiFiFailure;
    uint32_t ip;

    configASSERT( pcHost != NULL );
    configASSERT( pucIPAddr != NULL );

    ip = FreeRTOS_gethostbyname( pcHost );
    if( ip != 0 ) {
        memcpy( pucIPAddr, &ip, sizeof( uint32_t ) );
        ret = eWiFiSuccess;
    }

    return ret;
}

static WIFIScanResult_t *scan_results;
static uint8_t scan_result_max_count;
static int scan_count;

void sl_wfx_scan_result_callback( sl_wfx_scan_result_ind_body_t *scan_result )
{
    configASSERT( scan_result != NULL );

    if( scan_count < scan_result_max_count )
    {
        size_t ssid_len = scan_result->ssid_def.ssid_length;
        if( ssid_len > wificonfigMAX_SSID_LEN )
        {
            ssid_len = wificonfigMAX_SSID_LEN;
        }

        memcpy( scan_results[ scan_count ].cSSID, scan_result->ssid_def.ssid, ssid_len );
        memcpy( scan_results[ scan_count ].ucBSSID, scan_result->mac, wificonfigMAX_BSSID_LEN );
        scan_results[ scan_count ].cChannel = scan_result->channel;
        scan_results[ scan_count ].ucHidden = 0;
        scan_results[ scan_count ].cRSSI = ( ( int16_t ) scan_result->rcpi - 220 ) / 2;

        if( *( ( uint8_t * ) &scan_result->security_mode ) == 0 )
        {
            scan_results[ scan_count ].xSecurity = eWiFiSecurityOpen;
        }
        else
        {
            scan_results[ scan_count ].xSecurity = eWiFiSecurityNotSupported;

            if( scan_result->security_mode.wep )
            {
                scan_results[ scan_count ].xSecurity = eWiFiSecurityWEP;
            }

            if( scan_result->security_mode.wpa )
            {
                scan_results[ scan_count ].xSecurity = eWiFiSecurityWPA;
            }

            if( scan_result->security_mode.wpa2 )
            {
                if( scan_result->security_mode.psk )
                {
                    scan_results[ scan_count ].xSecurity = eWiFiSecurityWPA2;
                }
                else if( scan_result->security_mode.eap )
                {
                    scan_results[ scan_count ].xSecurity = eWiFiSecurityWPA2_ent;
                }
            }
        }

        scan_count++;
    }
}

void sl_wfx_scan_complete_callback( sl_wfx_fmac_status_t status )
{
  xEventGroupSetBits( sl_wfx_event_group, SL_WFX_SCAN_COMPLETE );
}

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t *pxBuffer,
                            uint8_t ucNumNetworks )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;
    const uint8_t channel_list[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13 };

    configASSERT( pxBuffer != NULL );

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        sl_status_t sl_ret;

        scan_results = pxBuffer;
        scan_result_max_count = ucNumNetworks;
        scan_count = 0;

        memset( pxBuffer, 0, sizeof( WIFIScanResult_t ) * ucNumNetworks );

        sl_ret = sl_wfx_send_scan_command( WFM_SCAN_MODE_ACTIVE,
                                           channel_list,
                                           SL_WFX_ARRAY_COUNT( channel_list ),
                                           NULL,
                                           0,
                                           NULL,
                                           0,
                                           NULL);
        if( sl_ret == SL_STATUS_OK )
        {
            bits = xEventGroupWaitBits( sl_wfx_event_group, SL_WFX_SCAN_COMPLETE, pdTRUE, pdTRUE, pdMS_TO_TICKS( 10000 ) );

            if( ( bits & SL_WFX_SCAN_COMPLETE ) == 0 )
            {
                ret = eWiFiTimeout;
            }
        }
        else
        {
            ret = eWiFiFailure;
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

void sl_wfx_start_ap_callback(sl_wfx_fmac_status_t status)
{
    if( status == WFM_STATUS_SUCCESS )
    {
        sl_wfx_host_log( "AP started\n" );
        sl_wfx_context->state |= SL_WFX_AP_INTERFACE_UP;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_START_AP );
    }
    else
    {
        sl_wfx_host_log( "AP start failed\n" );
        sl_wfx_context->state &= ~SL_WFX_AP_INTERFACE_UP;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_START_AP_FAIL );
    }
}

WIFIReturnCode_t WIFI_StartAP( void )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( wifi_ap_settings.configured == pdFALSE )
        {
            ret = eWiFiFailure;
        }

        if( ret == eWiFiSuccess )
        {
            ret = WIFI_SetMode( eWiFiModeAP );
        }

        if( ret == eWiFiSuccess )
        {
            ret = WIFI_StopAP();
        }

        if( ret == eWiFiSuccess )
        {
            sl_status_t sl_ret;

            /*
             * Ensure the start AP fail bit is cleared as it is not automatically
             * cleared below. The start AP bit should be cleared at this point.
             */
            bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_START_AP_FAIL );
            configASSERT( ( bits & SL_WFX_START_AP ) == 0 );

            sl_ret = sl_wfx_start_ap_command( wifi_ap_settings.channel,
                                              wifi_ap_settings.ssid,
                                              wifi_ap_settings.ssid_length,
                                              0, /* SSID is not hidden */
                                              0, /* Don't isolate clients */
                                              wifi_ap_settings.security,
                                              /* enable management frame protection if WPA */
                                              wifi_ap_settings.security == WFM_SECURITY_MODE_WPA2_PSK ||
                                              wifi_ap_settings.security == WFM_SECURITY_MODE_WPA2_WPA1_PSK ? 1 : 0,
                                              wifi_ap_settings.password,
                                              wifi_ap_settings.password_length,
                                              NULL, /* No vendor specific beacon data */
                                              0,
                                              NULL, /* No vendor specific probe data */
                                              0 );

            if( sl_ret == SL_STATUS_OK )
            {
                bits = xEventGroupWaitBits( sl_wfx_event_group,
                                            SL_WFX_START_AP | SL_WFX_START_AP_FAIL,
                                            pdFALSE, /* Do not clear these bits */
                                            pdFALSE,
                                            pdMS_TO_TICKS( SL_WFX_CONNECT_TIMEOUT_MS ) );

                if( ( bits & SL_WFX_START_AP ) == 0 )
                {
                    ret = eWiFiFailure;
                }
            }
            else
            {
                ret = eWiFiFailure;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

void sl_wfx_stop_ap_callback(void)
{
    sl_wfx_host_log( "SoftAP stopped\n" );
    sl_wfx_context->state &= ~SL_WFX_AP_INTERFACE_UP;
    xEventGroupClearBits( sl_wfx_event_group, SL_WFX_START_AP );
    xEventGroupSetBits( sl_wfx_event_group, SL_WFX_STOP_AP );
    FreeRTOS_NetworkDown();
}

WIFIReturnCode_t WIFI_StopAP( void )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( sl_wfx_context->state & SL_WFX_AP_INTERFACE_UP )
        {
            sl_status_t sl_ret;

            sl_ret = sl_wfx_stop_ap_command();

            if( sl_ret == SL_STATUS_OK )
            {
                bits = xEventGroupWaitBits( sl_wfx_event_group,
                                            SL_WFX_STOP_AP,
                                            pdTRUE,
                                            pdTRUE,
                                            pdMS_TO_TICKS( SL_WFX_DISCONNECT_TIMEOUT_MS ) );

                if( ( bits & SL_WFX_STOP_AP ) == 0 )
                {
                    ret = eWiFiFailure;
                }
            }
            else
            {
                ret = eWiFiFailure;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    WIFIReturnCode_t ret;

    configASSERT( pxNetworkParams != NULL );

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        switch (pxNetworkParams->xSecurity)
        {
        case eWiFiSecurityOpen:
            wifi_ap_settings.security = WFM_SECURITY_MODE_OPEN;
            break;
        case eWiFiSecurityWEP:
            wifi_ap_settings.security = WFM_SECURITY_MODE_WEP;
            break;
        case eWiFiSecurityWPA:
            wifi_ap_settings.security = WFM_SECURITY_MODE_WPA2_WPA1_PSK;
            break;
        case eWiFiSecurityWPA2:
            wifi_ap_settings.security = WFM_SECURITY_MODE_WPA2_PSK;
            break;
        default:
            ret = eWiFiFailure;
            break;
        }

        if( ret == eWiFiSuccess )
        {
            wifi_ap_settings.ssid[ 0 ] = '\0';
            strncat( ( char * ) wifi_ap_settings.ssid, pxNetworkParams->pcSSID, wificonfigMAX_SSID_LEN );
            wifi_ap_settings.ssid_length = pxNetworkParams->ucSSIDLength;

            wifi_ap_settings.password[ 0 ] = '\0';
            strncat( ( char * ) wifi_ap_settings.password, pxNetworkParams->pcPassword, wificonfigMAX_PASSPHRASE_LEN );
            wifi_ap_settings.password_length = pxNetworkParams->ucPasswordLength;

            wifi_ap_settings.channel = pxNetworkParams->cChannel;

            wifi_ap_settings.configured = pdTRUE;
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

/*
 * TODO: later?
 */
WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void *pvOptionValue )
{
    configASSERT( pvOptionValue != NULL );
    return eWiFiNotSupported;
}

/*
 * TODO: later?
 */
WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t *pxPMModeType,
                                 void * pvOptionValue )
{
    configASSERT( pxPMModeType != NULL );
    configASSERT( pvOptionValue != NULL );
    return eWiFiNotSupported;
}

BaseType_t WIFI_IsConnected( void )
{
    BaseType_t ret = pdFALSE;

    if( WIFI_GetLock() == eWiFiSuccess )
    {
        EventBits_t bits;
        bits = xEventGroupGetBits( sl_wfx_event_group );
        if( ( bits & ( SL_WFX_CONNECT | SL_WFX_START_AP ) ) != 0 )
        {
            ret = pdTRUE;
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

/*
 * TODO: Make this a function that the application's
 * xApplicationDHCPHook() calls.
 */
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
    WIFIDeviceMode_t mode;
    ulIPAddress = FreeRTOS_ntohl( ulIPAddress );

    WIFI_GetMode( &mode );

    if( mode == SL_WFX_SOFTAP_INTERFACE )
    {
        sl_wfx_host_log( "DHCP client not used in SoftAP mode\n" );
        sl_wfx_host_log( "Using default IP address %d.%d.%d.%d\n", ( ulIPAddress >> 24 ) & 0xff, ( ulIPAddress >> 16 ) & 0xff, ( ulIPAddress >> 8 ) & 0xff, ( ulIPAddress >> 0 ) & 0xff );
        return eDHCPUseDefaults;
    }
    else
    {
        if( eDHCPPhase == eDHCPPhasePreRequest )
        {
            sl_wfx_host_log("DHCP assigned IP address %d.%d.%d.%d\n", ( ulIPAddress >> 24 ) & 0xff, ( ulIPAddress >> 16 ) & 0xff, ( ulIPAddress >> 8 ) & 0xff, ( ulIPAddress >> 0 ) & 0xff );
        }
        return eDHCPContinue;
    }
}
