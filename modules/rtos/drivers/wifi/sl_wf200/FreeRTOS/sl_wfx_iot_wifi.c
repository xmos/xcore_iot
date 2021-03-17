// Copyright 2019 XMOS LIMITED. This Software is subject to the terms of the 
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT IOT_WIFI

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"
#include "FreeRTOS_DHCP.h"

#include "sl_wfx.h"
#include "FreeRTOS/sl_wfx_host.h"
#include "brd8022a_pds.h"
#include "brd8023a_pds.h"

/* SW services headers */
#include "FreeRTOS/sl_wfx_iot_wifi.h"
#if USE_DHCPD
#include "dhcpd.h"
#endif
#if USE_FATFS
#include "ff.h"
#endif

#define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define HWADDR_ARG(hwaddr) (hwaddr)[0], (hwaddr)[1], (hwaddr)[2], (hwaddr)[3], (hwaddr)[4], (hwaddr)[5]

#define SL_WFX_CONNECT_TIMEOUT_MS 10500
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
            if( sl_wfx_event_group != NULL )
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
            }
            else
            {
                xSemaphoreGiveRecursive( wifi_lock );
                ret = eWiFiFailure;
            }
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

#if USE_FATFS
static char *pds_file_load(char **pds, int *line_count)
{
    FRESULT result;
    FIL wf200_pds_file;
    uint32_t wf200_pds_size = -1;
    UINT bytes_read = 0;
    int max_lines = *line_count;
    char *pds_data = NULL;

    *line_count = 0;

    result = f_open(&wf200_pds_file, "/flash/firmware/wf200pds.dat", FA_READ);
    if (result == FR_OK) {
    	wf200_pds_size = f_size(&wf200_pds_file);
    	pds_data = pvPortMalloc(wf200_pds_size);
    }
    if (pds_data != NULL) {
    	result = f_read(&wf200_pds_file, pds_data, wf200_pds_size, &bytes_read);
    }
	if (result == FR_OK && bytes_read == wf200_pds_size) {
		char *saveptr;
		char *line;

		line = strtok_r(pds_data, "\r\n", &saveptr);
		while (line != NULL) {
			pds[(*line_count)++] = line;
			if (*line_count >= max_lines) {
				break;
			}
			line = strtok_r(NULL, "\r\n", &saveptr);
		}
	}

	if (wf200_pds_size != -1) {
		f_close(&wf200_pds_file);
	}
	if (*line_count == 0 && pds_data != NULL) {
		vPortFree(pds_data);
		pds_data = NULL;
	}

	return pds_data;
}
#endif

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

        if( xSemaphoreTakeRecursive( wifi_lock, pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS ) ) == pdPASS )
        {
#if USE_FATFS
            char *pds_data;
			char *pds[10];
			int line_count = 10;

			pds_data = pds_file_load( pds, &line_count );

			if( line_count > 0 ) {
				rtos_printf("Loading %d lines of WF200 PDS data from filesystem:\n", line_count);
				sl_wfx_host_set_pds( (const char **) pds, line_count );
				for (int i = 0; i < line_count; i++) {
					rtos_printf("  %s\n", pds[i]);
				}
			}
			else
#endif
			{
#if XCOREAI_EXPLORER || XCORE200_MAB
				rtos_printf("WF200 PDS data not found in filesystem.\nUsing brd8023a PDS data\n");
				sl_wfx_host_set_pds( pds_table_brd8023a, SL_WFX_ARRAY_COUNT( pds_table_brd8023a ) );
#elif OSPREY_BOARD
				rtos_printf("WF200 PDS data not found in filesystem.\nUsing brd8022a PDS data\n");
				sl_wfx_host_set_pds( pds_table_brd8022a, SL_WFX_ARRAY_COUNT( pds_table_brd8022a ) );
#endif
			}

            sl_ret = sl_wfx_init( &wfx_ctx );

            if( sl_ret == SL_STATUS_OK )
            {
                wifi_mode = eWiFiModeStation;
                ret = eWiFiSuccess;
            }

            xSemaphoreGiveRecursive( wifi_lock );

#if USE_FATFS
            if( pds_data != NULL )
            {
            	vPortFree( pds_data );
            }
#endif
        }
    }

    return ret;
}

WIFIReturnCode_t WIFI_Off( void )
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( sl_wfx_context->state & SL_WFX_STARTED )
        {
            sl_status_t sl_ret;
            sl_ret = sl_wfx_deinit();
            if( sl_ret != SL_STATUS_OK )
            {
                ret = eWiFiFailure;
            }
        }

        /* Ensure all state bits are cleared */
        sl_wfx_context->state = 0;
        xEventGroupClearBits( sl_wfx_event_group, 0x00FFFFFF );

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

void sl_wfx_disconnect_callback(uint8_t *mac, sl_wfx_disconnected_reason_t reason)
{
    EventBits_t bits;
    sl_wfx_host_log( "Disconnected from AP " HWADDR_FMT " for reason %d\n", HWADDR_ARG( mac ), reason );
    sl_wfx_context->state &= ~SL_WFX_STA_INTERFACE_CONNECTED;
    bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_CONNECT );
    xEventGroupSetBits( sl_wfx_event_group, SL_WFX_DISCONNECT );

    if( ( bits & SL_WFX_CONNECT ) != 0 )
    {
        rtos_printf("Bringing the FreeRTOS network down\n");
        FreeRTOS_NetworkDown();
    }
}

WIFIReturnCode_t WIFI_Disconnect( void )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( ( xEventGroupGetBits( sl_wfx_event_group ) & SL_WFX_CONNECT ) != 0 )
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
                    /*
                     * Timed out waiting for the callback. We are now likely out of
                     * sync with the device. Reset it to get back to a known state.
                     */
                    WIFI_Reset();
                    ret = eWiFiTimeout;
                }
            }
            else
            {
                if( sl_ret == SL_STATUS_WIFI_WRONG_STATE)
                {
                    WIFI_Reset();
                }
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

    switch ( status )
    {
    case WFM_STATUS_SUCCESS:
        sl_wfx_host_log( "Connection succeeded to AP " HWADDR_FMT "\n", HWADDR_ARG( mac ) );
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
        sl_wfx_host_log( "Connection attempt error %x\n", status );
        break;
    }

    if( connected )
    {
        sl_wfx_context->state |= SL_WFX_STA_INTERFACE_CONNECTED;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_CONNECT );
        xEventGroupClearBits( sl_wfx_event_group, SL_WFX_DISCONNECT );
    }
    else
    {
        EventBits_t bits;

        sl_wfx_context->state &= ~SL_WFX_STA_INTERFACE_CONNECTED;

        bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_CONNECT );
        if( ( bits & SL_WFX_CONNECT ) != 0 )
        {
            /*
             * This can happen if an autonomous roaming connection attempt fails.
             *
             * The module was already connected, but now it is not. This means that
             * WIFI_ConnectAP() is likely not waiting, since it ensures that the
             * module is disconnected first if the connect flag was already set,
             * thus ensuring it is cleared. Bring the FreeRTOS TCP stack down, which
             * will likely cause the application to attempt to reconnect.
             */
            rtos_printf("Bringing the FreeRTOS network down\n");
            FreeRTOS_NetworkDown();
        }

        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_CONNECT_FAIL );
    }
}

static const sl_wfx_mac_address_t *connect_bssid;

WIFIReturnCode_t WIFI_ConnectAPSetBSSID(const uint8_t *bssid)
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        connect_bssid = (sl_wfx_mac_address_t *) bssid;

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    sl_wfx_security_mode_t security;
    EventBits_t bits;
    WIFIReturnCode_t ret;
    uint8_t prevent_roaming = 0;

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
            if( ( xEventGroupGetBits( sl_wfx_event_group ) & SL_WFX_CONNECT ) != 0 )
            {
                ret = WIFI_Disconnect();
            }
        }

        if( ret == eWiFiSuccess )
        {
            sl_status_t sl_ret;

            if( connect_bssid != NULL )
            {
                /* Explicitly prevent roaming if the BSSID of the AP is specified */
                prevent_roaming = 1;
                rtos_printf( "Connect to: %s(%d, " HWADDR_FMT "):%s(%d) on ch %d with security %d\n",
                             pxNetworkParams->pcSSID,
                             pxNetworkParams->ucSSIDLength,
                             HWADDR_ARG(connect_bssid->octet),
                             pxNetworkParams->pcPassword,
                             pxNetworkParams->ucPasswordLength,
                             pxNetworkParams->cChannel,
                             security );
            }
            else
            {
                rtos_printf( "Connect to: %s(%d):%s(%d) on ch %d with security %d\n",
                             pxNetworkParams->pcSSID,
                             pxNetworkParams->ucSSIDLength,
                             pxNetworkParams->pcPassword,
                             pxNetworkParams->ucPasswordLength,
                             pxNetworkParams->cChannel,
                             security );
            }


            /*
             * Ensure the connect fail bit is cleared as it is not automatically
             * cleared below. The connect bit should be cleared at this point.
             */
            bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_CONNECT_FAIL );
            configASSERT( ( bits & SL_WFX_CONNECT ) == 0 );

            sl_ret = sl_wfx_send_join_command( ( const uint8_t * ) pxNetworkParams->pcSSID,
                                               pxNetworkParams->ucSSIDLength,
                                               connect_bssid,
                                               pxNetworkParams->cChannel,
                                               security,
                                               prevent_roaming,
                                               /* enable management frame protection if WPA */
                                               security == WFM_SECURITY_MODE_WPA2_PSK ||
                                               security == WFM_SECURITY_MODE_WPA2_WPA1_PSK ? WFM_MGMT_FRAME_PROTECTION_OPTIONAL : WFM_MGMT_FRAME_PROTECTION_DISABLED,
                                               ( const uint8_t * ) pxNetworkParams->pcPassword,
                                               pxNetworkParams->ucPasswordLength,
                                               NULL,
                                               0);
            connect_bssid = NULL;

            if( sl_ret == SL_STATUS_OK )
            {
                bits = xEventGroupWaitBits( sl_wfx_event_group,
                                            SL_WFX_CONNECT | SL_WFX_CONNECT_FAIL,
                                            pdFALSE, /* Do not clear these bits */
                                            pdFALSE,
                                            pdMS_TO_TICKS( SL_WFX_CONNECT_TIMEOUT_MS ) );

                if( ( bits & SL_WFX_CONNECT ) == 0 )
                {
                    if( ( bits & SL_WFX_CONNECT_FAIL ) == 0 )
                    {
                        /*
                         * Timed out waiting for the callback. We are now likely out of
                         * sync with the device. Reset it to get back to a known state.
                         */
                        WIFI_Reset();
                    }
                    ret = eWiFiTimeout;
                }
            }
            else
            {
                if( sl_ret == SL_STATUS_WIFI_WRONG_STATE)
                {
                    WIFI_Reset();
                }
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

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t *pusIndex )
{
#if USE_FATFS && FF_FS_MINIMIZE == 0 && !FF_FS_READONLY
    int opened = 0;
    WIFIReturnCode_t ret = eWiFiFailure;
    FRESULT result;
    FIL networks;
    FSIZE_t pos;

    configASSERT( pxNetworkProfile != NULL );

    result = f_open(&networks, "/flash/wifi/networks.dat", FA_OPEN_APPEND | FA_WRITE);

    if (result == FR_OK) {
        opened = 1;
        pos = f_tell(&networks);
        if (pos % sizeof(WIFINetworkProfile_t) != 0) {
            /*
             * If the size of the networks file is not a multiple of the size of
             * the network profile struct, then reset the file.
             */
            result = f_lseek(&networks, 0);
            if (result == FR_OK) {
                result = f_truncate(&networks);
                pos = 0;
            }
        }
    }

    if (result == FR_OK) {
        UINT bytes_written;
        result = f_write(&networks, pxNetworkProfile, sizeof(WIFINetworkProfile_t), &bytes_written);
        if (result == FR_OK) {
            if (bytes_written == sizeof(WIFINetworkProfile_t)) {
                if (pusIndex != NULL) {
                    *pusIndex = pos / sizeof(WIFINetworkProfile_t);
                }
                ret = eWiFiSuccess;
            } else {
                /* The write succeeded, but was unable to write the entire profile. Undo. */
                result = f_lseek(&networks, pos);
                if (result == FR_OK) {
                    (void) f_truncate(&networks);
                }
            }
        }
    }

    if (opened) {
        (void) f_close(&networks);
    }

    return ret;
#else
    return eWiFiNotSupported;
#endif
}

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t *pxNetworkProfile,
                                  uint16_t usIndex )
{
#if USE_FATFS
    WIFIReturnCode_t ret = eWiFiFailure;
    FRESULT result;
    FIL networks;
    FSIZE_t size;

    configASSERT( pxNetworkProfile != NULL );

    result = f_open(&networks, "/flash/wifi/networks.dat", FA_READ);

    if (result == FR_OK) {
        size = f_size(&networks);
        if (size % sizeof(WIFINetworkProfile_t) == 0) {
            FSIZE_t pos = usIndex * sizeof(WIFINetworkProfile_t);
            if (pos < size) {
                UINT bytes_read;

                /*
                 * The size of the networks file is a multiple of the size of the
                 * network profile struct and the requested index is within the file.
                 */

                if (
                f_lseek(&networks, pos) == FR_OK &&
                f_read(&networks, pxNetworkProfile, sizeof(WIFINetworkProfile_t), &bytes_read) == FR_OK &&
                bytes_read == sizeof(WIFINetworkProfile_t)) {
                    ret = eWiFiSuccess;
                }
            }
        }

        (void) f_close(&networks);
    }

    return ret;
#else
    return eWiFiNotSupported;
#endif
}

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
#if USE_FATFS && FF_FS_MINIMIZE == 0 && !FF_FS_READONLY
    WIFIReturnCode_t ret = eWiFiFailure;
    FRESULT result;
    FIL networks;
    FSIZE_t size;

    result = f_open(&networks, "/flash/wifi/networks.dat", FA_READ | FA_WRITE);

    if (result == FR_OK) {
        size = f_size(&networks);
        if (size % sizeof(WIFINetworkProfile_t) == 0) {
            FSIZE_t dest = usIndex * sizeof(WIFINetworkProfile_t);
            FSIZE_t src = dest + sizeof(WIFINetworkProfile_t);
            WIFINetworkProfile_t tmp;
            if (dest < size) {
                UINT bytes_read;
                UINT bytes_written;

                /*
                 * The size of the networks file is a multiple of the size of the
                 * network profile struct and the requested index is within the file.
                 */

                while (result == FR_OK && src < size) {
                    if (
                    (result = f_lseek(&networks, src)) == FR_OK &&
                    (result = f_read(&networks, &tmp, sizeof(WIFINetworkProfile_t), &bytes_read)) == FR_OK &&
                    (result = f_lseek(&networks, dest)) == FR_OK &&
                    (result = f_write(&networks, &tmp, sizeof(WIFINetworkProfile_t), &bytes_written)) == FR_OK) {
                        src += sizeof(WIFINetworkProfile_t);
                        dest += sizeof(WIFINetworkProfile_t);
                    }
                }

                if (
                result == FR_OK &&
                f_lseek(&networks, dest) == FR_OK &&
                f_truncate(&networks) == FR_OK) {
                    ret = eWiFiSuccess;
                }
            }
        }

        (void) f_close(&networks);
    }

    return ret;
#else
    return eWiFiNotSupported;
#endif
}

#if ( ipconfigSUPPORT_OUTGOING_PINGS == 1 )
__attribute__((weak))
void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier)
{
	if (eStatus == eSuccess) {
		#if USE_DHCPD
			dhcpd_ping_reply_received( usIdentifier );
		#endif
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
#endif

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
        scan_results[ scan_count ].ucHidden = ssid_len == 0 ? 1 : 0;
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

static const sl_wfx_ssid_def_t *scan_search_list;
static int scan_search_list_count;

WIFIReturnCode_t WIFI_ScanSetSSIDList(const sl_wfx_ssid_def_t *list, int count)
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        scan_search_list = list;
        scan_search_list_count = count;

        WIFI_ReleaseLock();
    }

    return ret;
}

static const sl_wfx_mac_address_t *scan_bssid;

WIFIReturnCode_t WIFI_ScanSetBSSID(const uint8_t *bssid)
{
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        scan_bssid = (sl_wfx_mac_address_t *) bssid;

        WIFI_ReleaseLock();
    }

    return ret;
}

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t *pxBuffer,
                            uint8_t ucNumNetworks )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

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
                                           NULL,
                                           0,
                                           scan_search_list,
                                           scan_search_list_count,
                                           NULL,
                                           0,
                                           (uint8_t *) scan_bssid);

        scan_bssid = NULL;

        if( sl_ret == SL_STATUS_OK || sl_ret == SL_STATUS_WIFI_WARNING )
        {
            bits = xEventGroupWaitBits( sl_wfx_event_group, SL_WFX_SCAN_COMPLETE, pdTRUE, pdTRUE, pdMS_TO_TICKS( 10000 ) );

            if( ( bits & SL_WFX_SCAN_COMPLETE ) == 0 )
            {
                ret = eWiFiTimeout;
            }
        }
        else
        {
            sl_wfx_host_log( "sl_wfx_send_scan_command() returned %04x\n", sl_ret );
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
        sl_wfx_host_log( "SoftAP started\n" );
        sl_wfx_context->state |= SL_WFX_AP_INTERFACE_UP;
        xEventGroupSetBits( sl_wfx_event_group, SL_WFX_START_AP );
        xEventGroupClearBits( sl_wfx_event_group, SL_WFX_STOP_AP );
    }
    else
    {
        EventBits_t bits;

        sl_wfx_host_log( "AP start failed\n" );
        sl_wfx_context->state &= ~SL_WFX_AP_INTERFACE_UP;

        bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_START_AP );
        if( ( bits & SL_WFX_START_AP ) != 0 )
        {
            /*
             * This is not supposed to be able to happen, but handling the case
             * just in case.
             *
             * The module had already started a soft AP, but now it has stopped.
             * This means that WIFI_StartAP() is likely not waiting, since it ensures
             * that the soft AP is stopped first if the start AP flag was already set,
             * thus ensuring it is cleared. Bring the FreeRTOS TCP stack down, which
             * will likely cause the application to attempt to restart the soft AP.
             */
            rtos_printf("Bringing the FreeRTOS network down\n");
            FreeRTOS_NetworkDown();
        }

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
            if( ( xEventGroupGetBits( sl_wfx_event_group ) & SL_WFX_START_AP ) != 0 )
            {
                ret = WIFI_StopAP();
            }
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
                                              wifi_ap_settings.security == WFM_SECURITY_MODE_WPA2_WPA1_PSK ? WFM_MGMT_FRAME_PROTECTION_OPTIONAL : WFM_MGMT_FRAME_PROTECTION_DISABLED,
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
                    if( ( bits & SL_WFX_START_AP_FAIL ) == 0 )
                    {
                        /*
                         * Timed out waiting for the callback. We are now likely out of
                         * sync with the device. Reset it to get back to a known state.
                         */
                        WIFI_Reset();
                    }
                    ret = eWiFiTimeout;
                }
            }
            else
            {
                if( sl_ret == SL_STATUS_WIFI_WRONG_STATE)
                {
                    WIFI_Reset();
                }
                ret = eWiFiFailure;
            }
        }

        WIFI_ReleaseLock();
    }

    return ret;
}

void sl_wfx_stop_ap_callback(void)
{
    EventBits_t bits;
    sl_wfx_host_log( "SoftAP stopped\n" );
    sl_wfx_context->state &= ~SL_WFX_AP_INTERFACE_UP;
    bits = xEventGroupClearBits( sl_wfx_event_group, SL_WFX_START_AP );
    xEventGroupSetBits( sl_wfx_event_group, SL_WFX_STOP_AP );

    if( ( bits & SL_WFX_START_AP ) != 0 )
    {
        rtos_printf("Bringing the FreeRTOS network down\n");
        FreeRTOS_NetworkDown();
    }
}

WIFIReturnCode_t WIFI_StopAP( void )
{
    EventBits_t bits;
    WIFIReturnCode_t ret;

    ret = WIFI_GetLock();
    if( ret == eWiFiSuccess )
    {
        if( ( xEventGroupGetBits( sl_wfx_event_group ) & SL_WFX_START_AP ) != 0 )
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
                    /*
                     * Timed out waiting for the callback. We are now likely out of
                     * sync with the device. Reset it to get back to a known state.
                     */
                    WIFI_Reset();
                    ret = eWiFiTimeout;
                }
            }
            else
            {
                if( sl_ret == SL_STATUS_WIFI_WRONG_STATE)
                {
                    WIFI_Reset();
                }
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

int WIFI_DHCPHook( int eDHCPPhase,
                   uint32_t ulIPAddress )
{
    WIFIDeviceMode_t mode;
    ulIPAddress = FreeRTOS_ntohl( ulIPAddress );

    WIFI_GetMode( &mode );

    if( mode == SL_WFX_SOFTAP_INTERFACE )
    {
        rtos_printf( "DHCP client not used in SoftAP mode\n" );
        rtos_printf( "Using default IP address %d.%d.%d.%d\n", ( ulIPAddress >> 24 ) & 0xff, ( ulIPAddress >> 16 ) & 0xff, ( ulIPAddress >> 8 ) & 0xff, ( ulIPAddress >> 0 ) & 0xff );
        return eDHCPUseDefaults;
    }
    else
    {
        if( eDHCPPhase == eDHCPPhasePreRequest )
        {
            rtos_printf("DHCP assigned IP address %d.%d.%d.%d\n", ( ulIPAddress >> 24 ) & 0xff, ( ulIPAddress >> 16 ) & 0xff, ( ulIPAddress >> 8 ) & 0xff, ( ulIPAddress >> 0 ) & 0xff );
        }
        return eDHCPContinue;
    }
}

__attribute__((weak))
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
    return WIFI_DHCPHook( eDHCPPhase, ulIPAddress );
}

/**************************************************************************//**
 * Callback for client connect to AP
 *****************************************************************************/
void sl_wfx_client_connected_callback(uint8_t* mac)
{
    sl_wfx_host_log("Client connected, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**************************************************************************//**
 * Callback for client rejected from AP
 *****************************************************************************/
void sl_wfx_ap_client_rejected_callback(sl_wfx_reason_t reason, uint8_t* mac)
{
    sl_wfx_host_log("Client rejected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         reason, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**************************************************************************//**
 * Callback for AP client disconnect
 *****************************************************************************/
void sl_wfx_ap_client_disconnected_callback(sl_wfx_reason_t reason, uint8_t* mac)
{
    sl_wfx_host_log("Client disconnected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         reason, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
