// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// This Software is subject to the terms of the XMOS Public Licence: Version 1.


#include <stdlib.h>
#include <string.h>

#include "sl_wfx.h"
#include "FreeRTOS/sl_wfx_host.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

#define printf rtos_printf

#define SL_WFX_EVENT_MAX_SIZE   512
#define SL_WFX_EVENT_LIST_SIZE  1

extern SemaphoreHandle_t s_xDriverSemaphore;

static QueueHandle_t eventQueue;

typedef struct {
    const char * const *pds_data;
    int firmware_index;
    uint8_t firmware_data[DOWNLOAD_BLOCK_SIZE];
    uint16_t pds_size;
    uint8_t waited_event_id;
} sl_wfx_host_ctx_t;

static sl_wfx_host_ctx_t host_ctx;

void sl_wfx_host_gpio(int gpio,
                      int value);

/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_pds(const char * const pds_data[],
                         uint16_t pds_size)
{
    host_ctx.pds_data = pds_data;
    host_ctx.pds_size = pds_size;
}

void sl_wfx_host_reset(void)
{
    sl_wfx_context->state &= ~SL_WFX_STARTED;
    sl_wfx_host_disable_platform_interrupt();
    sl_wfx_host_hold_in_reset();
    sl_wfx_deinit_bus();
    sl_wfx_reset_request_callback();
}

/**** XCORE Specific Functions End ****/

/**** Application Provided Functions ****
 *
 * These are
 * default implementations in case the application
 * does not provide them.
 */

__attribute__((weak))
void sl_wfx_reset_request_callback(void)
{
    while (sl_wfx_init(sl_wfx_context) != SL_STATUS_OK) {
        sl_wfx_host_wait(100);
    }

    /*
     * Ensure the application knows that it is no longer connected to
     * an AP in case it was.
     */
    sl_wfx_disconnect_callback((uint8_t *) "\x00\x00\x00\x00\x00\x00", WFM_DISCONNECTED_REASON_UNSPECIFIED);
}

#include "sl_wfx_wf200_C0.h"

__attribute__((weak))
uint32_t sl_wfx_app_fw_size(void)
{
	return sl_wfx_firmware_size;
}

__attribute__((weak))
sl_status_t sl_wfx_app_fw_read(uint8_t *data, uint32_t index, uint32_t size)
{
	if (index + size <= sl_wfx_firmware_size) {
		memcpy(data, &sl_wfx_firmware[index], size);
		return SL_STATUS_OK;
	} else {
		return SL_STATUS_FAIL;
	}
}

/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init(void)
{
    host_ctx.firmware_index = 0;
    if (eventQueue == NULL) {
        eventQueue = xQueueCreate(SL_WFX_EVENT_LIST_SIZE, sizeof(uint8_t));
        configASSERT(eventQueue != NULL);
    }
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_firmware_data(const uint8_t **data, uint32_t data_size)
{
	sl_status_t ret;

	ret = sl_wfx_app_fw_read(host_ctx.firmware_data, host_ctx.firmware_index, data_size);

	if (ret == SL_STATUS_OK) {
		host_ctx.firmware_index += data_size;
		*data = host_ctx.firmware_data;
	}

    return ret;
}


sl_status_t sl_wfx_host_get_firmware_size(uint32_t *firmware_size)
{
    *firmware_size = sl_wfx_app_fw_size();
    if (*firmware_size == 0) {
    	return SL_STATUS_NOT_AVAILABLE;
    }

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
    host_ctx.waited_event_id = 0;

    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state)
{
    if (state != 0) {
        /*
         * Presumably the WFX is asleep. Ensure the interrupt
         * event bit is low now before waking it up so that
         * sl_wfx_host_wait_for_wake_up() may wait for it to
         * be set by the ISR.
         */
        xEventGroupClearBits(sl_wfx_event_group, SL_WFX_WAKEUP);
    } else {
        rtos_printf("going to sleep\n");
    }
    sl_wfx_host_gpio(SL_WFX_HIF_GPIO_WUP, state);

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_wake_up(void)
{
    EventBits_t bits;

    bits = xEventGroupWaitBits(sl_wfx_event_group, SL_WFX_WAKEUP, pdTRUE, pdTRUE, pdMS_TO_TICKS(3));

    if (bits & SL_WFX_WAKEUP) {
        rtos_printf("woke up\n");
    } else {
        rtos_printf("did not wake\n");
    }

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_sleep_grant(sl_wfx_host_bus_transfer_type_t type,
                                    sl_wfx_register_address_t address,
                                    uint32_t length)
{
    return SL_STATUS_WIFI_SLEEP_GRANTED;
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
    BaseType_t ret;

    /* Wait for an event posted by the function sl_wfx_host_post_event() */
    ret = xQueueReceive(eventQueue, &posted_event_id, pdMS_TO_TICKS(timeout_ms));
    if (ret == pdTRUE) {
        /* Once a message is received, check if it is the expected ID */
        if (confirmation_id == posted_event_id) {
            /* Pass the confirmation reply and return*/
            if (event_payload_out != NULL) {
                *event_payload_out = sl_wfx_context->event_payload_buffer;
            }
            return SL_STATUS_OK;
        } else {
            sl_wfx_host_log("confirmation id is %02x, expected %02x\n", (int) posted_event_id, (int) confirmation_id);
        }
    } else {
        sl_wfx_host_log("sl_wfx_host_wait_for_confirmation() queue recv returned %d waiting for id %02X\n", ret, confirmation_id);
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
          sl_wfx_exception_ind_t *firmware_exception = (sl_wfx_exception_ind_t*)event_payload;
          uint8_t *exception_tmp = (uint8_t *) firmware_exception;
          printf("firmware exception %lu\r\n", firmware_exception->body.reason);
          for (uint16_t i = 0; i < firmware_exception->header.length; i += 16) {
            printf("hif: %.8x:", i);
            for (uint8_t j = 0; (j < 16) && ((i + j) < firmware_exception->header.length); j ++) {
                printf(" %.2x", *exception_tmp);
                exception_tmp++;
            }
            printf("\r\n");
          }
          sl_wfx_host_reset();
          break;
        }
      case SL_WFX_ERROR_IND_ID:
        {
          sl_wfx_error_ind_t *firmware_error = (sl_wfx_error_ind_t*)event_payload;
          uint8_t *error_tmp = (uint8_t *) firmware_error;
          printf("firmware error %lu\r\n", firmware_error->body.type);
          for (uint16_t i = 0; i < firmware_error->header.length; i += 16) {
            printf("hif: %.8x:", i);
            for (uint8_t j = 0; (j < 16) && ((i + j) < firmware_error->header.length); j ++) {
                printf(" %.2x", *error_tmp);
                error_tmp++;
            }
            printf("\r\n");
          }
          sl_wfx_host_reset();
          break;
        }
      }

      if(host_ctx.waited_event_id && (host_ctx.waited_event_id == event_payload->header.id))
      {
        xassert(event_payload->header.length <= SL_WFX_EVENT_MAX_SIZE);

        host_ctx.waited_event_id = 0;

        /* Post the event in the queue */
        memcpy( sl_wfx_context->event_payload_buffer,
               (void*) event_payload,
               event_payload->header.length );
        xQueueOverwrite(eventQueue, (void *) &event_payload->header.id);
      }

      return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_allocate_buffer(void **buffer,
                                        sl_wfx_buffer_type_t type,
                                        uint32_t buffer_size)
{
/*
 * Zero copy RX is no longer supported. This code is being left in place
 * for the future if support for these is ever added back in.
 */
#if 0
    if (ipconfigZERO_COPY_RX_DRIVER != 0 && type == SL_WFX_RX_FRAME_BUFFER) {
        NetworkBufferDescriptor_t *network_buffer;
        network_buffer = pxGetNetworkBufferWithDescriptor( buffer_size, 0 );
        if (network_buffer != NULL) {
            /*
             * The pointer that will be returned to the driver is behind the beginning
             * of the Ethernet Frame by the size of the received indication message and
             * SL_WFX_NORMAL_FRAME_PAD_LENGTH padding bytes. This way frames with 2 pad
             * bytes will begin at where pucEthernetBuffer points to.
             *
             * When the padding is other than SL_WFX_NORMAL_FRAME_PAD_LENGTH bytes then
             * this needs to be dealt with in the receive callback.
             *
             * This requires that the FreeRTOS+TCP option ipBUFFER_PADDING be large enough
             * to hold the sl_wfx_received_ind_t message.
             */
            *buffer = network_buffer->pucEthernetBuffer - sizeof(sl_wfx_received_ind_t) - SL_WFX_NORMAL_FRAME_PAD_LENGTH;
        } else {
            *buffer = NULL;
        }
    } else {
        *buffer = pvPortMalloc(buffer_size);
    }
#else
    *buffer = pvPortMalloc(buffer_size);
#endif

    if (buffer != NULL) {
        return SL_STATUS_OK;
    } else {
        return SL_STATUS_NO_MORE_RESOURCE;
    }
}

sl_status_t sl_wfx_host_free_buffer(void *buffer, sl_wfx_buffer_type_t type)
{
//    if (ipconfigZERO_COPY_RX_DRIVER == 0 || type != SL_WFX_RX_FRAME_BUFFER) {
        vPortFree(buffer);
//    }

    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_transmit_frame(void *frame, uint32_t frame_len)
{
    return sl_wfx_data_write(frame, frame_len);
}

sl_status_t sl_wfx_host_lock(void)
{
    sl_status_t status;

    if (xSemaphoreTakeRecursive(s_xDriverSemaphore, pdMS_TO_TICKS(SL_WFX_DEFAULT_REQUEST_TIMEOUT_MS)) == pdTRUE) {
        status = SL_STATUS_OK;
    } else {
        printf("Wi-Fi driver mutex timeout\r\n");
        status = SL_STATUS_TIMEOUT;
    }

    return status;
}

sl_status_t sl_wfx_host_unlock(void)
{
	if (xSemaphoreGetMutexHolder(s_xDriverSemaphore) == xTaskGetCurrentTaskHandle()) {
	    xSemaphoreGiveRecursive(s_xDriverSemaphore);
	}

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
