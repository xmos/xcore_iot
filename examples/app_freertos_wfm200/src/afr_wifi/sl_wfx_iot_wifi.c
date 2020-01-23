/*
 * sl_wfx_iot_wifi.c
 *
 *  Created on: Jan 21, 2020
 *      Author: mbruno
 */

#ifndef SL_WFX_IOT_WIFI_C_
#define SL_WFX_IOT_WIFI_C_

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

//#include "FreeRTOS_IP.h"
//#include "FreeRTOS_Sockets.h"
//#include "FreeRTOS_DHCP.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"
#include "brd8023a_pds.h"

#include "sl_wfx_iot_wifi.h"

#define SL_WFX_CONNECT_TIMEOUT_MS 10000
#define SL_WFX_DISCONNECT_TIMEOUT_MS 10000

static sl_wfx_context_t wfx_ctx;

static SemaphoreHandle_t wifi_lock;

WIFIReturnCode_t WIFI_GetLock( void )
{
    if( wifi_lock != NULL )
    {
        if( xSemaphoreTakeRecursive( wifi_lock, pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS ) ) == pdPASS )
        {
            return eWiFiSuccess;
        }
        else
        {
            return eWiFiTimeout;
        }
    }
    else
    {
        return eWiFiFailure;
    }
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
    sl_status_t ret;

    if (sl_wfx_context != NULL && sl_wfx_context->state & SL_WFX_STARTED) {
        return eWiFiSuccess;
    }

    /*
     * TODO: Perhaps protect with mutex in case the application and
     * xNetworkInterfaceInitialise() both call at the same time.
     * Or don't have xNetworkInterfaceInitialise() call it.
     */

    sl_wfx_host_set_hif(bitstream_spi_devices[BITSTREAM_SPI_DEVICE_A],
                        bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A],
                        gpio_1I, 0,  /* header pin 9 */
                        gpio_1P, 0,  /* header pin 10 */
                        gpio_1J, 0); /* header pin 12 */

    sl_wfx_host_set_pds(pds_table_brd8023a, SL_WFX_ARRAY_COUNT(pds_table_brd8023a));

    ret = sl_wfx_init(&wfx_ctx);

    if (ret == SL_STATUS_OK) {
        if (wifi_lock == NULL) {
            wifi_lock = xSemaphoreCreateRecursiveMutex();
            xassert(wifi_lock != NULL);
        }
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_INITIALIZED);
        return eWiFiSuccess;
    } else {
        return eWiFiFailure;
    }
}

WIFIReturnCode_t WIFI_Off( void )
{
    WIFI_GetLock();
    sl_wfx_shutdown();
    WIFI_ReleaseLock();
    /* TODO: Test */
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_Disconnect( void )
{
    EventBits_t bits;

    if( WIFI_GetLock() == eWiFiSuccess )
    {
        if (sl_wfx_context->state & SL_WFX_STA_INTERFACE_CONNECTED) {
            sl_wfx_send_disconnect_command();
            bits = xEventGroupWaitBits(sl_wfx_event_group,
                                       SL_WFX_DISCONNECT,
                                       pdTRUE,
                                       pdTRUE,
                                       pdMS_TO_TICKS(SL_WFX_DISCONNECT_TIMEOUT_MS));

            if (bits & SL_WFX_DISCONNECT) {
                WIFI_ReleaseLock();
                return eWiFiSuccess;
            } else {
                WIFI_ReleaseLock();
                return eWiFiFailure;
            }
        } else {
            /* Already disconnected */
            WIFI_ReleaseLock();
            return eWiFiSuccess;
        }
    }
    else
    {
        return eWiFiTimeout;
    }
}

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    sl_status_t ret;
    sl_wfx_security_mode_t security;
    EventBits_t bits;

    switch (pxNetworkParams->xSecurity) {
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
        return eWiFiFailure;
    }

    if( WIFI_GetLock() == eWiFiSuccess )
    {
        if (WIFI_Disconnect() == eWiFiFailure) {
            WIFI_ReleaseLock();
            return eWiFiFailure;
        }

        sl_wfx_host_log(
                "Connect to: %s(%d):%s(%d) on ch %d with security %d\n",
                pxNetworkParams->pcSSID,
                pxNetworkParams->ucSSIDLength,
                pxNetworkParams->pcPassword,
                pxNetworkParams->ucPasswordLength,
                pxNetworkParams->cChannel,
                security);

        /*
         * Ensure the connect fail bit is set as it is not automatically
         * cleared below. The connect bit should be cleared at this point.
         */
        bits = xEventGroupClearBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        xassert((bits & SL_WFX_CONNECT) == 0);

        ret = sl_wfx_send_join_command((const uint8_t *) pxNetworkParams->pcSSID,
                                       pxNetworkParams->ucSSIDLength,
                                       NULL,
                                       pxNetworkParams->cChannel,
                                       security,
                                       0,
                                       1,
                                       (const uint8_t *) pxNetworkParams->pcPassword,
                                       pxNetworkParams->ucPasswordLength,
                                       NULL,
                                       0);

        if (ret != SL_STATUS_OK) {
            WIFI_ReleaseLock();
            return eWiFiFailure;
        }

        bits = xEventGroupWaitBits(sl_wfx_event_group,
                                   SL_WFX_CONNECT | SL_WFX_CONNECT_FAIL,
                                   pdFALSE, /* Do not clear these bits */
                                   pdFALSE,
                                   pdMS_TO_TICKS(SL_WFX_CONNECT_TIMEOUT_MS));

        if (bits & SL_WFX_CONNECT) {
            WIFI_ReleaseLock();
            return eWiFiSuccess;
        } else {
            WIFI_ReleaseLock();
            return eWiFiFailure;
        }
    }
    else
    {
        return eWiFiTimeout;
    }
}

WIFIReturnCode_t WIFI_Reset( void )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    return eWiFiNotSupported;
}

static WIFIScanResult_t *scan_results;
static uint8_t scan_result_max_count;
static int scan_count;

void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t* scan_result)
{
    if (scan_count < scan_result_max_count) {
        size_t ssid_len = scan_result->ssid_def.ssid_length;
        if (ssid_len > wificonfigMAX_SSID_LEN) {
            ssid_len = wificonfigMAX_SSID_LEN;
        }
        memcpy(scan_results[scan_count].cSSID, scan_result->ssid_def.ssid, ssid_len);
        memcpy(scan_results[scan_count].ucBSSID, scan_result->mac, wificonfigMAX_BSSID_LEN);
        scan_results[scan_count].cChannel = scan_result->channel;
        scan_results[scan_count].ucHidden = 0;
        scan_results[scan_count].cRSSI = ((int16_t) scan_result->rcpi - 220) / 2;

        if (*((uint8_t *) &scan_result->security_mode) == 0) {
            scan_results[scan_count].xSecurity = eWiFiSecurityOpen;
        } else {

            scan_results[scan_count].xSecurity = eWiFiSecurityNotSupported;

            if (scan_result->security_mode.wep) {
                scan_results[scan_count].xSecurity = eWiFiSecurityWEP;
            }

            if (scan_result->security_mode.wpa) {
                scan_results[scan_count].xSecurity = eWiFiSecurityWPA;
            }

            if (scan_result->security_mode.wpa2) {
                if (scan_result->security_mode.psk) {
                    scan_results[scan_count].xSecurity = eWiFiSecurityWPA2;
                } else if (scan_result->security_mode.eap) {
                    scan_results[scan_count].xSecurity = eWiFiSecurityWPA2_ent;
                }
            }
        }

        scan_count++;
    }
}

void sl_wfx_scan_complete_callback(uint32_t status)
{
  xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_COMPLETE);
}

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    EventBits_t bits;
    const uint8_t channel_list[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};

    if( WIFI_GetLock() == eWiFiSuccess )
    {

        scan_results = pxBuffer;
        scan_result_max_count = ucNumNetworks;
        scan_count = 0;

        memset(pxBuffer, 0, sizeof(WIFIScanResult_t) * ucNumNetworks);

        sl_wfx_send_scan_command(WFM_SCAN_MODE_ACTIVE,
                                 channel_list,
                                 SL_WFX_ARRAY_COUNT(channel_list),
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 NULL);

        bits = xEventGroupWaitBits(sl_wfx_event_group, SL_WFX_SCAN_COMPLETE, pdTRUE, pdTRUE, pdMS_TO_TICKS(1000));

        if (bits & SL_WFX_SCAN_COMPLETE) {
            WIFI_ReleaseLock();
            return eWiFiSuccess;
        } else {
            WIFI_ReleaseLock();
            return eWiFiTimeout;
        }
    }
    else
    {
        return eWiFiTimeout;
    }
}

WIFIReturnCode_t WIFI_StartAP( void )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_StopAP( void )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    return eWiFiNotSupported;
}

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    return eWiFiNotSupported;
}

BaseType_t WIFI_IsConnected( void )
{
    return eWiFiNotSupported;
}

#endif /* SL_WFX_IOT_WIFI_C_ */
