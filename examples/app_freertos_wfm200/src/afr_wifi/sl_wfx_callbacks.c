/*
 * sl_wfx_callbacks.c
 *
 *  Created on: Jan 21, 2020
 *      Author: mbruno
 */

#include <stdlib.h>
#include <string.h>

#include "sl_wfx.h"
#include "sl_wfx_host.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define printf rtos_printf

#define SL_WFX_MAX_STATIONS     8
#define SL_WFX_MAX_SCAN_RESULTS 50

char event_log[50];
scan_result_list_t scan_list[SL_WFX_MAX_SCAN_RESULTS];
uint8_t scan_count = 0;
uint8_t scan_count_web = 0;



/**************************************************************************//**
 * Callback when station connects
 *****************************************************************************/
void sl_wfx_connect_callback(uint8_t* mac, uint32_t status)
{
    switch (status) {
    case WFM_STATUS_SUCCESS:
        sl_wfx_host_log("Connected\r\n");
        sl_wfx_context->state |= SL_WFX_STA_INTERFACE_CONNECTED;
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT);
        break;

    case WFM_STATUS_NO_MATCHING_AP:
        sl_wfx_host_log("Connection failed, access point not found\n");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;

    case WFM_STATUS_CONNECTION_ABORTED:
        sl_wfx_host_log("Connection aborted");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;

    case WFM_STATUS_CONNECTION_TIMEOUT:
        sl_wfx_host_log("Connection timeout");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;

    case WFM_STATUS_CONNECTION_REJECTED_BY_AP:
        sl_wfx_host_log("Connection rejected by the access point");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;

    case WFM_STATUS_CONNECTION_AUTH_FAILURE:
        sl_wfx_host_log("Connection authentication failure");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;

    default:
        sl_wfx_host_log("Connection attempt error");
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT_FAIL);
        break;
    }
}

/**************************************************************************//**
 * Callback for station disconnect
 *****************************************************************************/
void sl_wfx_disconnect_callback(uint8_t* mac, uint16_t reason)
{
    sl_wfx_host_log("Disconnected %d\r\n", reason);
    sl_wfx_context->state &= ~SL_WFX_STA_INTERFACE_CONNECTED;
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_DISCONNECT);
}

/**************************************************************************//**
 * Callback for AP started
 *****************************************************************************/
void sl_wfx_start_ap_callback(uint32_t status)
{
  if (status == 0) {
    printf("AP started\r\n");
//    printf("Join the AP with SSID: %s\r\n", softap_ssid);
    sl_wfx_context->state |= SL_WFX_AP_INTERFACE_UP;
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_START_AP);
  } else {
    printf("AP start failed\r\n");
    strcpy(event_log, "AP start failed");
  }
}

/**************************************************************************//**
 * Callback for AP stopped
 *****************************************************************************/
void sl_wfx_stop_ap_callback(void)
{
//  dhcpserver_clear_stored_mac ();
  printf("SoftAP stopped\r\n");
  sl_wfx_context->state &= ~SL_WFX_AP_INTERFACE_UP;
  xEventGroupSetBits(sl_wfx_event_group, SL_WFX_STOP_AP);
}

/**************************************************************************//**
 * Callback for client connect to AP
 *****************************************************************************/
void sl_wfx_client_connected_callback(uint8_t* mac)
{
  printf("Client connected, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//  printf("Open a web browser and go to http://%d.%d.%d.%d\r\n",
//         AP_IP_ADDR0_DEFAULT, AP_IP_ADDR1_DEFAULT, AP_IP_ADDR2_DEFAULT, AP_IP_ADDR3_DEFAULT);
}

/**************************************************************************//**
 * Callback for client rejected from AP
 *****************************************************************************/
void sl_wfx_ap_client_rejected_callback(uint32_t status, uint8_t* mac)
{
//  struct eth_addr mac_addr;
//  memcpy(&mac_addr, mac, SL_WFX_BSSID_SIZE);
//  dhcpserver_remove_mac(&mac_addr);
  printf("Client rejected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         (int)status, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**************************************************************************//**
 * Callback for AP client disconnect
 *****************************************************************************/
void sl_wfx_ap_client_disconnected_callback(uint32_t status, uint8_t* mac)
{
//  struct eth_addr mac_addr;
//  memcpy(&mac_addr, mac, SL_WFX_BSSID_SIZE);
//  dhcpserver_remove_mac(&mac_addr);
  printf("Client disconnected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         (int)status, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t* rx_buffer)
{
    rtos_printf("RX Frame!\n");
}
