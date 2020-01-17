// Copyright (c) 2020, XMOS Ltd, All rights reserved


#include <stdlib.h>
#include <string.h>

#include "sl_wfx.h"

#include "FreeRTOS.h"
#include "task.h"

static const char *pds_data_g;
static uint16_t pds_size_g;

/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_pds(const char *pds_data, uint16_t pds_size)
{
    pds_data_g = pds_data;
    pds_size_g = pds_size;
}

/**** XCORE Specific Functions End ****/


/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_firmware_data(const uint8_t **data, uint32_t data_size)
{
    return SL_STATUS_OK;
}


sl_status_t sl_wfx_host_get_firmware_size(uint32_t *firmware_size)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_data(const char **pds_data, uint16_t index)
{
    if (pds_data_g == NULL) {
        return SL_STATUS_FAIL;
    }

    *pds_data = pds_data_g;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_get_pds_size(uint16_t *pds_size)
{
    if (pds_size_g == 0) {
        return SL_STATUS_FAIL;
    }

    *pds_size = pds_size_g;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_deinit(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_reset_chip(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_wake_up(void)
{
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
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_setup_waited_event(uint8_t event_id)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait_for_confirmation(uint8_t confirmation_id,
                                              uint32_t timeout_ms,
                                              void **event_payload_out)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_wait(uint32_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_event(sl_wfx_generic_message_t *event_payload)
{
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
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_lock(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_unlock(void)
{
    return SL_STATUS_OK;
}

/**** WF200 Driver Required Host Functions End ****/
