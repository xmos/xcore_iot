// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT TUSB_DCD
#define DEBUG_PRINT_ENABLE_TUSB_DCD 0

#include "device/dcd.h"
#include "device/usbd.h" /* For tud_descriptor_configuration_cb() */

/*
 * By default, USB interrupts will run on core 0.
 */
#ifndef CFG_TUD_XCORE_INTERRUPT_CORE
#define CFG_TUD_XCORE_INTERRUPT_CORE 0
#endif

/*
 * By default, SOF interrupts will run on the same core
 * as the other USB interrupts.
 */
#ifndef CFG_TUD_XCORE_SOF_INTERRUPT_CORE
#define CFG_TUD_XCORE_SOF_INTERRUPT_CORE CFG_TUD_XCORE_INTERRUPT_CORE
#endif

/*
 * By default, the USB I/O thread may run on any
 * core other than 0.
 */
#ifndef CFG_TUD_XCORE_IO_CORE_MASK
#define CFG_TUD_XCORE_IO_CORE_MASK (~(1 << 0))
#endif

TU_ATTR_WEAK bool tud_xcore_sof_cb(uint8_t rhport);
TU_ATTR_WEAK bool tud_xcore_data_cb(uint32_t cur_time, uint32_t ep_num, uint32_t ep_dir, size_t xfer_len);

#include "rtos_usb.h"

static rtos_usb_t usb_ctx;

static union setup_packet_struct {
    tusb_control_request_t req;
    uint8_t pad[CFG_TUD_ENDPOINT0_SIZE]; /* In case an OUT data packet comes in instead of a SETUP packet */
} setup_packet;

static bool waiting_for_setup;

static void prepare_setup(bool in_isr)
{
    XUD_Result_t res;

//  rtos_printf("preparing for setup packet\n");
    waiting_for_setup = true;
    res = rtos_usb_endpoint_transfer_start(&usb_ctx, 0x00, (uint8_t *) &setup_packet, sizeof(setup_packet));

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
                                  rtos_usb_packet_type_t packet_type,
                                  XUD_Result_t res)
{
    /* Timestamp packets as they come in */

    uint32_t cur_time;
    asm volatile(
           "{gettime %0}"
           : "=r"(cur_time)
           : /* no resources*/
           : /* no clobbers */
           );

    if (res == XUD_RES_RST) {
        rtos_printf("Reset received on %02x\n", ep_address);
        reset_ep(ep_address, true);
        return;
    }

    // rtos_printf("packet rx'd, timestamp %d\n", cur_time); 

    switch (packet_type) {
    case rtos_usb_data_packet: {
        xfer_result_t tu_result;
        bool cb_result;

        if (res == XUD_RES_OKAY) {
            rtos_printf("xfer of %d bytes complete on %02x\n", xfer_len, ep_address);
            tu_result = XFER_RESULT_SUCCESS;

            if (ep_address == 0x00) {
                if (xfer_len == 0) {
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
                } else if (waiting_for_setup) {
                    /*
                     * We are waiting for a setup packet but OUT data on EP0 came in
                     * instead. This might be due to an unhandled SET request. In this
                     * case just drop the data and prepare for the next setup packet.
                     */
                    prepare_setup(true);
                    rtos_printf("Dropped unhandled OUT packet on EP0\n");
                    return;
                }
            }
        } else {
            rtos_printf("xfer on %02x failed with status %d\n", ep_address, res);
            tu_result = XFER_RESULT_FAILED;
            xfer_len = 0;
        }

        if (tud_xcore_data_cb) {
            uint32_t ep_num = tu_edpt_number(ep_address);
            uint32_t ep_dir = tu_edpt_dir(ep_address);
            cb_result = tud_xcore_data_cb(cur_time, ep_num, ep_dir, xfer_len);
        }

        dcd_event_xfer_complete(0, ep_address, xfer_len, tu_result, true);
        break;
    }
    case rtos_usb_setup_packet:
        rtos_printf("Setup packet of %d bytes received on %02x\n", xfer_len, ep_address);
        waiting_for_setup = 0;
        dcd_event_setup_received(0, (uint8_t *) &setup_packet, true);
        break;
    case rtos_usb_sof_packet:
        if (tud_xcore_sof_cb) {
            if (tud_xcore_sof_cb(0)) {
                dcd_event_bus_signal(0, DCD_EVENT_SOF, true);
            }
        }
        break;
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
    rtos_usb_init(&usb_ctx,
                   CFG_TUD_XCORE_IO_CORE_MASK,
                   dcd_xcore_int_handler, NULL);

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
// This both enable interrupts, and causes the USB driver's
// low level thread to enter XUD_Main().
// It must be called from an RTOS thread.
// This function is called by usb_task() prior to entering
// tud_task().
void dcd_connect(uint8_t rhport)
{
    (void) rhport;

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
                   endpoint_count,
                   epTypeTableOut,
                   epTypeTableIn,
                   TUD_OPT_HIGH_SPEED ? XUD_SPEED_HS : XUD_SPEED_FS,
                   pwr,
                   CFG_TUD_XCORE_INTERRUPT_CORE,
                   CFG_TUD_XCORE_SOF_INTERRUPT_CORE);
}

// Disconnect by disabling internal pull-up resistor on D+/D-
// TODO: Someday this might be able to make XUD_Main() return.
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

void dcd_edpt_close_all (uint8_t rhport)
{
    (void) rhport;
    // TODO see dcd_edpt_close()
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

// Submit a transfer where is managed by FIFO, When complete dcd_event_xfer_complete() is invoked to notify the stack - optional, however, must be listed in usbd.c
bool dcd_edpt_xfer_fifo (uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
  (void) rhport;
  (void) ep_addr;
  (void) ff;
  (void) total_bytes;
  return false;
}

// Stall endpoint
void dcd_edpt_stall (uint8_t rhport, uint8_t ep_addr)
{
    (void) rhport;

    rtos_printf("STALLING EP %02x\n", ep_addr);
    rtos_usb_endpoint_stall_set(&usb_ctx, ep_addr);

    if (ep_addr == 0x00 && !waiting_for_setup) {
        prepare_setup(false);
    }
}

// clear stall, data toggle is also reset to DATA0
void dcd_edpt_clear_stall (uint8_t rhport, uint8_t ep_addr)
{
    (void) rhport;

    rtos_usb_endpoint_stall_clear(&usb_ctx, ep_addr);
}
