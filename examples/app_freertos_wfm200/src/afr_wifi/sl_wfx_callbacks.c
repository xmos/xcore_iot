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
void sl_wfx_ap_client_rejected_callback(sl_wfx_reason_t reason, uint8_t* mac)
{
//  struct eth_addr mac_addr;
//  memcpy(&mac_addr, mac, SL_WFX_BSSID_SIZE);
//  dhcpserver_remove_mac(&mac_addr);
  printf("Client rejected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         reason, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**************************************************************************//**
 * Callback for AP client disconnect
 *****************************************************************************/
void sl_wfx_ap_client_disconnected_callback(sl_wfx_reason_t reason, uint8_t* mac)
{
//  struct eth_addr mac_addr;
//  memcpy(&mac_addr, mac, SL_WFX_BSSID_SIZE);
//  dhcpserver_remove_mac(&mac_addr);
  printf("Client disconnected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
         reason, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
