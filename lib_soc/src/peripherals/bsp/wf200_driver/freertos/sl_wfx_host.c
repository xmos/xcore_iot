// Copyright (c) 2020, XMOS Ltd, All rights reserved


#include <stdlib.h>
#include <string.h>

#include "quadflashlib.h"

#include "sl_wfx.h"
#include "sl_wfx_wf200_C0.h"
#include "sl_wfx_host.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

#define printf rtos_printf

char event_log[50];

extern SemaphoreHandle_t s_xDriverSemaphore;

scan_result_list_t scan_list[SL_WFX_MAX_SCAN_RESULTS];
uint8_t scan_count = 0;
uint8_t scan_count_web = 0;
QueueHandle_t eventQueue;

typedef struct {
    const char **pds_data;
    int firmware_index;
    uint16_t pds_size;
    uint8_t firmware_data[DOWNLOAD_BLOCK_SIZE];
    uint8_t waited_event_id;
} sl_wfx_host_ctx_t;

static sl_wfx_host_ctx_t host_ctx;


/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_pds(const char *pds_data[],
                         uint16_t pds_size)
{
    host_ctx.pds_data = pds_data;
    host_ctx.pds_size = pds_size;
}




/**************************************************************************//**
 * Callback for individual scan result
 *****************************************************************************/
void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t* scan_result)
{
  scan_count++;
  printf(
    "# %2d %2d  %03d %02X:%02X:%02X:%02X:%02X:%02X  %s",
    scan_count,
    scan_result->channel,
    ((int16_t)(scan_result->rcpi - 220) / 2),
    scan_result->mac[0], scan_result->mac[1],
    scan_result->mac[2], scan_result->mac[3],
    scan_result->mac[4], scan_result->mac[5],
    scan_result->ssid_def.ssid);
  /*Report one AP information*/
  printf("\r\n");
  if (scan_count <= SL_WFX_MAX_SCAN_RESULTS) {
    scan_list[scan_count - 1].ssid_def = scan_result->ssid_def;
    scan_list[scan_count - 1].channel = scan_result->channel;
    scan_list[scan_count - 1].security_mode = scan_result->security_mode;
    scan_list[scan_count - 1].rcpi = scan_result->rcpi;
    memcpy(scan_list[scan_count - 1].mac, scan_result->mac, 6);
  }
}

/**************************************************************************//**
 * Callback for scan complete
 *****************************************************************************/
void sl_wfx_scan_complete_callback(uint32_t status)
{
  scan_count_web = scan_count;
  scan_count = 0;
  xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_COMPLETE);
}

/**************************************************************************//**
 * Callback when station connects
 *****************************************************************************/
void sl_wfx_connect_callback(uint8_t* mac, uint32_t status)
{
  switch(status){
  case WFM_STATUS_SUCCESS:
    {
      printf("Connected\r\n");
      sl_wfx_context->state |= SL_WFX_STA_INTERFACE_CONNECTED;
      xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT);
      break;
    }
  case WFM_STATUS_NO_MATCHING_AP:
    {
      strcpy(event_log, "Connection failed, access point not found");
      printf(event_log);
      printf("\r\n");
      break;
    }
  case WFM_STATUS_CONNECTION_ABORTED:
    {
      strcpy(event_log, "Connection aborted");
      printf(event_log);
      printf("\r\n");
      break;
    }
  case WFM_STATUS_CONNECTION_TIMEOUT:
    {
      strcpy(event_log, "Connection timeout");
      printf(event_log);
      printf("\r\n");
      break;
    }
  case WFM_STATUS_CONNECTION_REJECTED_BY_AP:
    {
      strcpy(event_log, "Connection rejected by the access point");
      printf(event_log);
      printf("\r\n");
      break;
    }
  case WFM_STATUS_CONNECTION_AUTH_FAILURE:
    {
      strcpy(event_log, "Connection authentication failure");
      printf(event_log);
      printf("\r\n");
      break;
    }
  default:
    {
      strcpy(event_log, "Connection attempt error");
      printf(event_log);
      printf("\r\n");
    }
  }
}

/**************************************************************************//**
 * Callback for station disconnect
 *****************************************************************************/
void sl_wfx_disconnect_callback(uint8_t* mac, uint16_t reason)
{
  printf("Disconnected %d\r\n", reason);
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

/**** XCORE Specific Functions End ****/


/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init(void)
{
    host_ctx.firmware_index = 0;
    eventQueue = xQueueCreate(SL_WFX_EVENT_LIST_SIZE, sizeof(uint8_t));
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_firmware_data(const uint8_t **data, uint32_t data_size)
{
    taskENTER_CRITICAL();
    fl_readData(host_ctx.firmware_index,
                data_size,
                host_ctx.firmware_data);
    taskEXIT_CRITICAL();

    host_ctx.firmware_index += data_size;
    *data = host_ctx.firmware_data;

    return SL_STATUS_OK;
}


sl_status_t sl_wfx_host_get_firmware_size(uint32_t *firmware_size)
{
    *firmware_size = sl_wfx_firmware_size;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_data(const char **pds_data, uint16_t index)
{
    if (host_ctx.pds_data == NULL) {
        return SL_STATUS_FAIL;
    }

    *pds_data = host_ctx.pds_data[index];
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_size(uint16_t *pds_size)
{
    if (host_ctx.pds_size == 0) {
        return SL_STATUS_FAIL;
    }

    *pds_size = host_ctx.pds_size;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_deinit(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_reset_chip(void)
{
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state)
{
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_WUP, state);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_wake_up(void)
{
    /*
     * TODO: what sets this..??
     */
    xEventGroupClearBits(sl_wfx_event_group, SL_WFX_INTERRUPT);
    xEventGroupWaitBits(sl_wfx_event_group, SL_WFX_INTERRUPT, pdTRUE, pdTRUE, pdMS_TO_TICKS(3));

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_sleep_grant(sl_wfx_host_bus_transfer_type_t type,
                                    sl_wfx_register_address_t address,
                                    uint32_t length)
{
    return SL_STATUS_WIFI_SLEEP_NOT_GRANTED;
}

sl_status_t sl_wfx_host_hold_in_reset(void)
{
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_RESET, 0);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_setup_waited_event(uint8_t event_id)
{
    host_ctx.waited_event_id = event_id;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_confirmation(uint8_t confirmation_id,
                                              uint32_t timeout_ms,
                                              void **event_payload_out)
{
    uint8_t posted_event_id;

    /* Wait for an event posted by the function sl_wfx_host_post_event() */
    if (xQueueReceive(eventQueue, &posted_event_id, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
        /* Once a message is received, check if it is the expected ID */
        if (confirmation_id == posted_event_id) {
            /* Pass the confirmation reply and return*/
            if (event_payload_out != NULL) {
                *event_payload_out = sl_wfx_context->event_payload_buffer;
            }
            return SL_STATUS_OK;
        }
    }

    /* The wait for the confirmation timed out, return */
    return SL_STATUS_TIMEOUT;
}

sl_status_t sl_wfx_host_wait(uint32_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_event(sl_wfx_generic_message_t *event_payload)
{
    switch(event_payload->header.id){
        /******** INDICATION ********/
      case SL_WFX_CONNECT_IND_ID:
        {
          sl_wfx_connect_ind_t* connect_indication = (sl_wfx_connect_ind_t*) event_payload;
          sl_wfx_connect_callback(connect_indication->body.mac, connect_indication->body.status);
          break;
        }
      case SL_WFX_DISCONNECT_IND_ID:
        {
          sl_wfx_disconnect_ind_t* disconnect_indication = (sl_wfx_disconnect_ind_t*) event_payload;
          sl_wfx_disconnect_callback(disconnect_indication->body.mac, disconnect_indication->body.reason);
          break;
        }
      case SL_WFX_START_AP_IND_ID:
        {
          sl_wfx_start_ap_ind_t* start_ap_indication = (sl_wfx_start_ap_ind_t*) event_payload;
          sl_wfx_start_ap_callback(start_ap_indication->body.status);
          break;
        }
      case SL_WFX_STOP_AP_IND_ID:
        {
          sl_wfx_stop_ap_callback();
          break;
        }
      case SL_WFX_RECEIVED_IND_ID:
        {
          sl_wfx_received_ind_t* ethernet_frame = (sl_wfx_received_ind_t*) event_payload;
          if ( ethernet_frame->body.frame_type == 0 )
          {
            sl_wfx_host_received_frame_callback( ethernet_frame );
          }
          break;
        }
      case SL_WFX_SCAN_RESULT_IND_ID:
        {
          sl_wfx_scan_result_ind_t* scan_result = (sl_wfx_scan_result_ind_t*) event_payload;
          sl_wfx_scan_result_callback(&scan_result->body);
          break;
        }
      case SL_WFX_SCAN_COMPLETE_IND_ID:
        {
          sl_wfx_scan_complete_ind_t* scan_complete = (sl_wfx_scan_complete_ind_t*) event_payload;
          sl_wfx_scan_complete_callback(scan_complete->body.status);
          break;
        }
      case SL_WFX_AP_CLIENT_CONNECTED_IND_ID:
        {
          sl_wfx_ap_client_connected_ind_t* client_connected_indication = (sl_wfx_ap_client_connected_ind_t*) event_payload;
          sl_wfx_client_connected_callback(client_connected_indication->body.mac);
          break;
        }
      case SL_WFX_AP_CLIENT_REJECTED_IND_ID:
        {
          sl_wfx_ap_client_rejected_ind_t* ap_client_rejected_indication = (sl_wfx_ap_client_rejected_ind_t*) event_payload;
          sl_wfx_ap_client_rejected_callback(ap_client_rejected_indication->body.reason, ap_client_rejected_indication->body.mac);
          break;
        }
      case SL_WFX_AP_CLIENT_DISCONNECTED_IND_ID:
        {
          sl_wfx_ap_client_disconnected_ind_t* ap_client_disconnected_indication = (sl_wfx_ap_client_disconnected_ind_t*) event_payload;
          sl_wfx_ap_client_disconnected_callback(ap_client_disconnected_indication->body.reason, ap_client_disconnected_indication->body.mac);
          break;
        }
      case SL_WFX_GENERIC_IND_ID:
        {
          break;
        }
      case SL_WFX_EXCEPTION_IND_ID:
        {
          printf("Firmware Exception\r\n");
          sl_wfx_exception_ind_t *firmware_exception = (sl_wfx_exception_ind_t*)event_payload;
          printf ("Exception data = ");
          for(int i = 0; i < SL_WFX_EXCEPTION_DATA_SIZE; i++)
          {
            printf ("%X, ", firmware_exception->body.data[i]);
          }
          printf("\r\n");
          break;
        }
      case SL_WFX_ERROR_IND_ID:
        {
          printf("Firmware Error\r\n");
          sl_wfx_error_ind_t *firmware_error = (sl_wfx_error_ind_t*)event_payload;
          printf ("Error type = %lu\r\n",firmware_error->body.type);
          break;
        }
      }

      if(host_ctx.waited_event_id == event_payload->header.id)
      {
        if(event_payload->header.length < SL_WFX_EVENT_MAX_SIZE)
        {
          /* Post the event in the queue */
          memcpy( sl_wfx_context->event_payload_buffer,
                 (void*) event_payload,
                 event_payload->header.length );
          xQueueOverwrite(eventQueue, (void *) &event_payload->header.id);
        }
      }

      return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_allocate_buffer(void **buffer,
                                        sl_wfx_buffer_type_t type,
                                        uint32_t buffer_size)
{
    /*
     * TODO: type could potentially be used
     * to determine which heap to allocate
     * from (SRAM or DDR)
     */
    (void) type;

    *buffer = pvPortMalloc(buffer_size);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_free_buffer(void *buffer, sl_wfx_buffer_type_t type)
{
    (void) type;

    vPortFree(buffer);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_transmit_frame(void *frame, uint32_t frame_len)
{
    return sl_wfx_data_write(frame, frame_len);
}

sl_status_t sl_wfx_host_lock(void)
{
    sl_status_t status;

    if (xSemaphoreTake(s_xDriverSemaphore, 500) == pdTRUE) {
        status = SL_STATUS_OK;
    } else {
        printf("Wi-Fi driver mutex timeout\r\n");
        status = SL_STATUS_TIMEOUT;
    }

    return status;
}

sl_status_t sl_wfx_host_unlock(void)
{
    xSemaphoreGive(s_xDriverSemaphore);

    return SL_STATUS_OK;
}

#if SL_WFX_DEBUG_MASK
void sl_wfx_host_log(const char *string, ...)
{
    va_list ap;

    va_start(ap, string);
    rtos_vprintf(string, ap);
    va_end(ap);
}
#endif

/**** WF200 Driver Required Host Functions End ****/


