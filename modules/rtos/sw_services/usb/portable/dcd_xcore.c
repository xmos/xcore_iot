// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <stdbool.h>
#include <rtos/drivers/usb/api/rtos_usb.h>

#include "tusb_option.h"

#include "device/dcd.h"


extern rtos_usb_t usb_ctx;

static uint32_t setup_packet[sizeof(tusb_control_request_t) / sizeof(uint32_t) + 1];

static void prepare_setup(bool in_isr)
{
    XUD_Result_t res;

    res = rtos_usb_endpoint_transfer_start(&usb_ctx, 0x00, (uint8_t *) setup_packet, sizeof(tusb_control_request_t));

    xassert(res == XUD_RES_OKAY);
//    if (res == XUD_RES_RST) {
//        reset_ep(0x00, in_isr);
//    }
}

static tusb_speed_t xud_to_tu_speed(XUD_BusSpeed_t xud_speed)
{

    switch (xud_speed) {
    case XUD_SPEED_FS:
        return TUSB_SPEED_FULL;
    case XUD_SPEED_HS:
        return TUSB_SPEED_HIGH;
    default:
        xassert(0);
        return TUSB_SPEED_INVALID;
    }
}

static void reset_ep(uint8_t ep_addr, bool in_isr)
{
    XUD_BusSpeed_t xud_speed;
    tusb_speed_t tu_speed;

    uint8_t const epnum = tu_edpt_number(ep_addr);
    uint8_t const rhport = 0;

    uint8_t dir = tu_edpt_dir(ep_addr);

    XUD_ep one = usb_ctx.ep[epnum][dir];
    XUD_ep *two = NULL;

    dir = dir ? 0 : 1;

    if (usb_ctx.ep[epnum][dir] != 0) {
        two = &usb_ctx.ep[epnum][dir];
    }

    xud_speed = XUD_ResetEndpoint(one, two);
    tu_speed = xud_to_tu_speed(xud_speed);

    prepare_setup(in_isr);

    dcd_event_bus_reset(rhport, tu_speed, in_isr);
}

/*------------------------------------------------------------------*/
/* Device API
 *------------------------------------------------------------------*/

// Initialize controller to device mode
void dcd_init(uint8_t rhport)
{
    /*
     * Is there any way that rtos_usb_start() could be called
     * here, so that the application doesn't have to call it
     * directly?
     *
     * It might/should be possible to parse the descriptors provided
     * by the application, returned by one or both of
     * tud_descriptor_configuration_cb() and tud_descriptor_device_cb(),
     * and then provide the correct arguments to rtos_usb_start() based
     * on these.
     */
    //rtos_usb_start(...);

    rtos_usb_all_endpoints_ready(&usb_ctx, RTOS_OSAL_WAIT_FOREVER);

    prepare_setup(false);

    (void) rhport;
}

// Enable device interrupt
void dcd_int_enable(uint8_t rhport)
{
    /* Not needed for the XCORE port */
    (void) rhport;
}

// Disable device interrupt
void dcd_int_disable(uint8_t rhport)
{
    /* Not needed for the XCORE port */
    (void) rhport;
}

// Receive Set Address request, mcu port must also include status IN response
void dcd_set_address(uint8_t rhport,
                     uint8_t dev_addr)
{
    (void) rhport;
    XUD_SetDevAddr(dev_addr);
}

// Wake up host
void dcd_remote_wakeup(uint8_t rhport)
{
    /* Not supported on XCORE */
    (void) rhport;
}

// Connect by enabling internal pull-up resistor on D+/D-
void dcd_connect(uint8_t rhport)
{
    /* This function appears to be unused by the stack or any example */
    /* Only called by tud_connect() which is not called by anything */
    (void) rhport;
}

// Disconnect by disabling internal pull-up resistor on D+/D-
void dcd_disconnect(uint8_t rhport)
{
    /* This function appears to be unused by the stack or any example */
    /* Only called by tud_disconnect() which is not called by anything */
    (void) rhport;
}

void dcd_xcore_int_handler(rtos_usb_t *ctx,
                           void *app_data,
                           uint32_t ep_address,
                           size_t xfer_len,
                           XUD_Result_t res)
{
    xfer_result_t tu_result;

    if (res == XUD_RES_OKAY) {
        tu_result = XFER_RESULT_SUCCESS;
    } else {
        tu_result = XFER_RESULT_FAILED;
        xfer_len = 0;
    }

    if (res == XUD_RES_RST) {
        reset_ep(ep_address, true);
    }

    dcd_event_xfer_complete(0, ep_address, xfer_len, tu_result, true);
}

//--------------------------------------------------------------------+
// Endpoint API
//--------------------------------------------------------------------+

// Invoked when a control transfer's status stage is complete.
// May help DCD to prepare for next control transfer, this API is optional.
void dcd_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const *request)
{
  (void) rhport;
  (void) request;

  prepare_setup(false);
}

// Configure endpoint's registers according to descriptor
bool dcd_edpt_open(uint8_t rhport,
                   tusb_desc_endpoint_t const *ep_desc)
{
    (void) rhport;

    XUD_ResetEpStateByAddr(ep_desc->bEndpointAddress);

    return true;
}

void dcd_edpt_close(uint8_t rhport,
                    uint8_t ep_addr)
{
    /*
     * Not sure what this should do.
     *
     * If this is called and then dcd_edpt_open() is called again,
     * then this may have to set the endpoint bit in the event group
     * if dcd_edpt_open() calls rtos_usb_endpoint_ready().
     */
}

// Submit a transfer, When complete dcd_event_xfer_complete() is invoked to notify the stack
bool dcd_edpt_xfer(uint8_t rhport,
                   uint8_t ep_addr,
                   uint8_t *buffer,
                   uint16_t total_bytes)
{
    XUD_Result_t res;
    static uint32_t dummy_zlp_word;

    if (buffer == NULL) {
        if (total_bytes == 0) {
            /*
             * lib_xud crashes if a NULL buffer is provided when
             * transferring a zero length buffer.
             */
            buffer = (uint8_t *) &dummy_zlp_word;
        }
    }

    xassert(buffer != NULL);

    res = rtos_usb_endpoint_transfer_start(&usb_ctx, ep_addr, buffer, total_bytes);
    if (res == XUD_RES_OKAY) {
        return true;
    }

    if (res == XUD_RES_RST) {
        reset_ep(ep_addr, false);
    }

    return false;
}

// Stall endpoint
void dcd_edpt_stall (uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  XUD_SetStallByAddr(ep_addr);
}

// clear stall, data toggle is also reset to DATA0
void dcd_edpt_clear_stall (uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  XUD_ClearStallByAddr(ep_addr);
}
