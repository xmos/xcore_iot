// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

#include "device/dcd.h"
#include "device/usbd.h" /* For tud_descriptor_configuration_cb() */

#define DEBUG_UNIT TUSB_DCD

#include <rtos/drivers/usb/api/rtos_usb.h>

static rtos_usb_t usb_ctx;

static struct setup_packet_struct {
    tusb_control_request_t req;
    uint32_t pad; /* just in case the transfer writes a CRC */
} setup_packet;

static void prepare_setup(bool in_isr)
{
    XUD_Result_t res;

    rtos_printf("preparing for setup packet\n");
    res = rtos_usb_endpoint_transfer_start(&usb_ctx, 0x00, (uint8_t *) &setup_packet, sizeof(tusb_control_request_t));

    xassert(res == XUD_RES_OKAY);

    /*
     * Calling reset_ep() here would result in recursion.
     * TODO: Is there a way to handle this case?
     */
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

    xud_speed = rtos_usb_endpoint_reset(&usb_ctx, ep_addr);
    tu_speed = xud_to_tu_speed(xud_speed);

    prepare_setup(in_isr);

    dcd_event_bus_reset(0, tu_speed, in_isr);
}

RTOS_USB_ISR_CALLBACK_ATTR
static void dcd_xcore_int_handler(rtos_usb_t *ctx,
                                  void *app_data,
                                  uint32_t ep_address,
                                  size_t xfer_len,
                                  int is_setup,
                                  XUD_Result_t res)
{
    if (res == XUD_RES_RST) {
        rtos_printf("Reset received on %02x\n", ep_address);
        reset_ep(ep_address, true);
    }

    if (is_setup) {
        rtos_printf("Setup packet of %d bytes received on %02x\n", xfer_len, ep_address);
        dcd_event_setup_received(0, (uint8_t *) &setup_packet, true);
    } else {
        xfer_result_t tu_result;

        if (res == XUD_RES_OKAY) {
            rtos_printf("xfer of %d bytes complete on %02x\n", xfer_len, ep_address);
            tu_result = XFER_RESULT_SUCCESS;

            if (xfer_len == 0 && ep_address == 0x00) {
                /*
                 * A ZLP has presumably been received on the output endpoint 0.
                 * Ensure lib_xud is ready for the next setup packet. Hopefully
                 * it does not come in prior to setting this up.
                 *
                 * TODO:
                 * Ideally this buffer would be prepared prior to receiving the ZLP,
                 * but it doesn't appear that this is currently possible to do
                 * with lib_xud. This is under investigation.
                 */
                prepare_setup(true);
            }
        } else {
            rtos_printf("xfer on %02x failed with status %d\n", ep_address, res);
            tu_result = XFER_RESULT_FAILED;
            xfer_len = 0;
        }

        /*
         * TODO: Should this be called when res is XUD_RES_RST?
         */
        dcd_event_xfer_complete(0, ep_address, xfer_len, tu_result, true);
    }
}

/*------------------------------------------------------------------*/
/* Device API
 *------------------------------------------------------------------*/

/*
 * Builds the endpoint tables required by lib XUD from
 * the first configuration descriptor provided by the
 * application.
 *
 * FIXME:
 * This is not compatible with multiple configurations.
 * lib_xud would need to first be stopped and then restarted to
 * load a different configuration.
 *
 * More than one configuration is almost never used, so this is
 * not really an issue.
 */
static int cfg_desc_parse(XUD_EpType *epTypeTableOut, XUD_EpType *epTypeTableIn, XUD_PwrConfig *pwr)
{

    const uint8_t *desc_buf;
    const tusb_desc_configuration_t *cfg_desc;
    size_t cur_index = 0;
    size_t total_length;
    uint8_t desc_len;
    int max_epnum = 0;

    desc_buf = tud_descriptor_configuration_cb(0);

    cfg_desc = (tusb_desc_configuration_t *) desc_buf;

    desc_len = cfg_desc->bLength;
    total_length = cfg_desc->wTotalLength;

    xassert(desc_len >= sizeof(tusb_desc_configuration_t));
    xassert(cfg_desc->bDescriptorType == TUSB_DESC_CONFIGURATION);

    if (cfg_desc->bmAttributes & TUSB_DESC_CONFIG_ATT_SELF_POWERED) {
        *pwr = XUD_PWR_SELF;
    } else {
        *pwr = XUD_PWR_BUS;
    }

    cur_index += desc_len;
    desc_buf += desc_len;

    /*
     * Iterate though each descriptor in the configuration.
     * Get the transfer type and direction from each endpoint
     * descriptor and populate the tables required by lib_xud
     * accordingly.
     */
    while (cur_index < total_length) {
        const tusb_desc_endpoint_t *ep_desc;
        ep_desc = (tusb_desc_endpoint_t *) desc_buf;
        desc_len = ep_desc->bLength;
        if (cur_index + desc_len <= total_length) {
            if (ep_desc->bDescriptorType == TUSB_DESC_ENDPOINT && desc_len >= sizeof(tusb_desc_endpoint_t)) {
                uint8_t epnum = tu_edpt_number(ep_desc->bEndpointAddress);
                uint8_t dir   = tu_edpt_dir(ep_desc->bEndpointAddress);
                tusb_xfer_type_t type = ep_desc->bmAttributes.xfer;
                XUD_EpTransferType xud_type = XUD_EPTYPE_DIS;

                xassert(epnum < RTOS_USB_ENDPOINT_COUNT_MAX);

                if (epnum > max_epnum) {
                    max_epnum = epnum;
                }

                switch (type) {
                case TUSB_XFER_CONTROL:
                    xud_type = XUD_EPTYPE_CTL;
                    break;
                case TUSB_XFER_ISOCHRONOUS:
                    xud_type = XUD_EPTYPE_ISO;
                    break;
                case TUSB_XFER_BULK:
                    xud_type = XUD_EPTYPE_BUL;
                    break;
                case TUSB_XFER_INTERRUPT:
                    xud_type = XUD_EPTYPE_INT;
                    break;
                }

                if (dir == TUSB_DIR_IN) {
                    rtos_printf("Enabling input endpoint %d with type %d\n", epnum, xud_type);
                    epTypeTableIn[epnum] = xud_type;
                } else {
                    rtos_printf("Enabling output endpoint %d with type %d\n", epnum, xud_type);
                    epTypeTableOut[epnum] = xud_type;
                }
            }
        }

        cur_index += desc_len;
        desc_buf += desc_len;
    }

    return max_epnum + 1;
}

// Initialize controller to device mode
void dcd_init(uint8_t rhport)
{
    int i;
    size_t endpoint_count;
    XUD_PwrConfig pwr;

    XUD_EpType epTypeTableOut[RTOS_USB_ENDPOINT_COUNT_MAX] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};
    XUD_EpType epTypeTableIn[RTOS_USB_ENDPOINT_COUNT_MAX]  = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE};

    for (i = 1; i < RTOS_USB_ENDPOINT_COUNT_MAX; i++) {
        epTypeTableOut[i] = XUD_EPTYPE_DIS;
        epTypeTableIn[i] = XUD_EPTYPE_DIS;
    }

    endpoint_count = cfg_desc_parse(epTypeTableOut, epTypeTableIn, &pwr);
    rtos_printf("Endpoint count is %d\n", endpoint_count);

    rtos_usb_start(&usb_ctx,
                   dcd_xcore_int_handler, NULL,
                   endpoint_count,
                   epTypeTableOut,
                   epTypeTableIn,
                   (CFG_TUSB_RHPORT0_MODE & OPT_MODE_HIGH_SPEED) ? XUD_SPEED_HS : XUD_SPEED_FS,
                   pwr,
                   configMAX_PRIORITIES - 1); /* TODO: configurable? */

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

    rtos_printf("Asked to set device address to %d. Will after ZLP status.\n", dev_addr);

    // Respond with ZLP status
    dcd_edpt_xfer(rhport, 0x80, NULL, 0);

    // DCD can only set address after status for this request is complete.
    // Do it in dcd_edpt0_status_complete().
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

//--------------------------------------------------------------------+
// Endpoint API
//--------------------------------------------------------------------+

// Invoked when a control transfer's status stage is complete.
// May help DCD to prepare for next control transfer, this API is optional.
void dcd_edpt0_status_complete(uint8_t rhport,
                               tusb_control_request_t const *request)
{
    (void) rhport;

    if (request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_DEVICE &&
            request->bmRequestType_bit.type == TUSB_REQ_TYPE_STANDARD &&
            request->bRequest == TUSB_REQ_SET_ADDRESS) {

        const unsigned dev_addr = request->wValue;
        rtos_printf("Setting device address to %d now\n", dev_addr);
        rtos_usb_device_address_set(&usb_ctx, dev_addr);
    }
}

// Configure endpoint's registers according to descriptor
bool dcd_edpt_open(uint8_t rhport,
                   tusb_desc_endpoint_t const *ep_desc)
{
    (void) rhport;

    rtos_usb_endpoint_state_reset(&usb_ctx, ep_desc->bEndpointAddress);

    return true;
}

void dcd_edpt_close(uint8_t rhport,
                    uint8_t ep_addr)
{
    /*
     * Not sure what this should do.
     *
     * What happens if a transfer was previously scheduled and
     * has not yet completed? Especially if this is an OUT
     * endpoint? Should this transfer somehow be canceled?
     * Does the interrupt need to be disabled?
     */
    rtos_usb_endpoint_state_reset(&usb_ctx, ep_addr);
}

// Submit a transfer, When complete dcd_event_xfer_complete() is invoked to notify the stack
bool dcd_edpt_xfer(uint8_t rhport,
                   uint8_t ep_addr,
                   uint8_t *buffer,
                   uint16_t total_bytes)
{
    XUD_Result_t res;
    static uint32_t dummy_zlp_word;

    rtos_printf("xfer of %d bytes requested on %02x\n", total_bytes, ep_addr);

    if (buffer == NULL) {
        if (total_bytes == 0) {
            /*
             * lib_xud crashes if a NULL buffer is provided when
             * transferring a zero length buffer.
             */
            rtos_printf("transferring ZLP on %02x\n", ep_addr);
            buffer = (uint8_t *) &dummy_zlp_word;
        }
    }

    xassert(buffer != NULL);

    /*
     * If this is requesting the transfer of a ZLP status, then ensure that
     * a buffer is ready for the next setup packet, as it may be transferred
     * immediately following completion of this transfer.
     */
    if (tu_edpt_number(ep_addr) == 0 && total_bytes == 0 && tu_edpt_dir(ep_addr) != setup_packet.req.bmRequestType_bit.direction) {
        if (ep_addr == 0x80) {
            /*
             * TODO: Ideally this would be prepared regardless of
             * the data phase direction. But it doesn't appear to be
             * possible to prepare lib_xud with a buffer for the next
             * setup packet prior to preparing it for receipt of a ZLP
             * status from the host.
             * See related TODO in dcd_xcore_int_handler(). This is
             * currently under investigation.
             */
            prepare_setup(false);
        }
    }

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

  prepare_setup(false);
  rtos_usb_endpoint_stall_set(&usb_ctx, ep_addr);
}

// clear stall, data toggle is also reset to DATA0
void dcd_edpt_clear_stall (uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  rtos_usb_endpoint_stall_clear(&usb_ctx, ep_addr);
}
