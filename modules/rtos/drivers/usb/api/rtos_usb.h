// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_USB_H_
#define RTOS_USB_H_

/**
 * \addtogroup rtos_usb_driver rtos_usb_driver
 *
 * The public API for using the RTOS USB driver.
 * @{
 */

#include <xcore/channel.h>
#include <xud.h>
#include <xud_device.h>

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * The maximum number of USB endpoint numbers supported by the RTOS USB driver.
 */
#define RTOS_USB_ENDPOINT_COUNT_MAX 12


/**
 * @{
 * This is used to index into the second dimension of many of the
 * RTOS USB driver's endpoint arrays.
 */
#define RTOS_USB_OUT_EP 0
#define RTOS_USB_IN_EP  1
/**@}*/

/**
 * This attribute must be specified on the RTOS USB interrupt callback function
 * provided by the application.
 */
#define RTOS_USB_ISR_CALLBACK_ATTR __attribute__((fptrgroup("rtos_usb_isr_cb_fptr_grp")))

typedef enum {
    rtos_usb_data_packet,
    rtos_usb_setup_packet,
    rtos_usb_sof_packet
} rtos_usb_packet_type_t;

/**
 * Typedef to the RTOS USB driver instance struct.
 */
typedef struct rtos_usb_struct rtos_usb_t;

/**
 * Function pointer type for application provided RTOS USB interrupt callback function.
 *
 * This callback function is called when there is a USB transfer interrupt.
 *
 * \param ctx           A pointer to the associated USB driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param ep_address    The address of the USB endpoint that the transfer
 *                      has completed on.
 * \param xfer_len      The length of the data transferred.
 * \param packet_type   The type of packet transferred. See rtos_usb_packet_type_t.
 * \param res           The result of the transfer. See XUD_Result_t.
 */
typedef void (*rtos_usb_isr_cb_t)(rtos_usb_t *ctx, void *app_data, uint32_t ep_address, size_t xfer_len, rtos_usb_packet_type_t packet_type, XUD_Result_t res);

/**
 * Struct to hold USB transfer state data per endpoint, used
 * as the argument to the ISR.
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct {
    /** A pointer to the associated RTOS USB driver instance. */
    rtos_usb_t *usb_ctx;
    /** The requested transfer length - either the maximum length for
    OUT transfers, or the actual length for IN transfers */
    size_t len;
    /** The endpoint address for the transfer */
    uint8_t ep_address;
    /** The direction of the transfer. Either RTOS_USB_OUT_EP or RTOS_USB_IN_EP */
    uint8_t dir;
    /** The endpoint number (lower 4 bits of the endpoint address) */
    uint8_t ep_num;
    /** The result of the transfer */
    int8_t res;
} rtos_usb_ep_xfer_info_t;

/**
 * Struct representing an RTOS USB driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_usb_struct {
    size_t endpoint_count;
    chanend_t c_ep_out_xud[RTOS_USB_ENDPOINT_COUNT_MAX];
    chanend_t c_ep_in_xud[RTOS_USB_ENDPOINT_COUNT_MAX];
    chanend_t c_sof_xud;
    chanend_t c_sof;
    int sof_interrupt_enabled;

    XUD_EpType endpoint_out_type[RTOS_USB_ENDPOINT_COUNT_MAX];
    XUD_EpType endpoint_in_type[RTOS_USB_ENDPOINT_COUNT_MAX];
    XUD_PwrConfig power_source;
    XUD_BusSpeed_t speed;

    chanend_t c_ep[RTOS_USB_ENDPOINT_COUNT_MAX][2];
    XUD_ep ep[RTOS_USB_ENDPOINT_COUNT_MAX][2];
    int reset_received;
    rtos_osal_thread_t hil_thread;
    RTOS_USB_ISR_CALLBACK_ATTR rtos_usb_isr_cb_t isr_cb;
    void *isr_app_data;
    rtos_usb_ep_xfer_info_t ep_xfer_info[RTOS_USB_ENDPOINT_COUNT_MAX][2];
};

/**
 * Checks to see if a particular endpoint is ready to use.
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to check.
 * \param timeout       The maximum amount of time to wait for the endpoint to
 *                      become ready before returning.
 *
 * \retval XUD_RES_OKAY if the endpoint is ready to use.
 * \retval XUD_RES_ERR  if the endpoint is not ready to use.
 */
int rtos_usb_endpoint_ready(rtos_usb_t *ctx,
                            uint32_t endpoint_addr,
                            unsigned timeout);

/**
 * Checks to see if all endpoints are ready to use.
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param timeout       The maximum amount of time to wait for all endpoints to
 *                      become ready before returning.
 *
 * \retval XUD_RES_OKAY if all the endpoints are ready to use.
 * \retval XUD_RES_ERR  if not all the endpoints are ready to use.
 */
XUD_Result_t rtos_usb_all_endpoints_ready(rtos_usb_t *ctx,
                                          unsigned timeout);

/**
 * Requests a transfer on a USB endpoint. This function returns immediately.
 * When the transfer is complete, the application's ISR callback provided to
 * rtos_usb_start() will be called.
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to perform the transfer on.
 * \param buffer        A pointer to the buffer to transfer data into for OUT
 *                      endpoints, or from for IN endpoints.
 * \param len           The maximum number of bytes to receive for OUT endpoints,
 *                      or the actual number of bytes to send for IN endpoints.
 *
 * \retval XUD_RES_OKAY if the transfer was requested successfully.
 * \retval XUD_RES_RST  if the transfer was not requested and the USB bus needs
 *                      to be reset. In this case, the application should reset
 *                      the USB bus.
 */
XUD_Result_t rtos_usb_endpoint_transfer_start(rtos_usb_t *ctx,
                                              uint32_t endpoint_addr,
                                              uint8_t *buffer,
                                              size_t len);

/**
 * This function will complete a reset on an endpoint. The address of the endpoint
 * to reset must be provided, and may be either direction (IN or OUT) endpoint. If
 * there is an associated endpoint of the opposite direction, however, it will also
 * be reset.
 *
 * The return value should be inspected to find the new bus-speed.

 * \param  endpoint_addr IN or OUT endpoint address to reset.
 *
 * \retval XUD_SPEED_HS  the host has accepted that this device can execute at high speed.
 * \retval XUD_SPEED_FS  the device is running at full speed.

 */
XUD_BusSpeed_t rtos_usb_endpoint_reset(rtos_usb_t *ctx,
                                       uint32_t endpoint_addr);

/**
 * Sets the USB device's bus address. This function must be called after a
 * ``setDeviceAddress`` request is made by the host, and after the ZLP status
 * is sent.
 *
 * \param ctx  A pointer to the USB driver instance to use.
 * \param addr The device address requested by the host.
 */
static inline XUD_Result_t rtos_usb_device_address_set(rtos_usb_t *ctx,
                                                       uint32_t addr)
{
    (void) ctx;
    return XUD_SetDevAddr(addr);
}

/**
 * Reset a USB endpoint's state including data PID toggle.
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to reset.
 */
static inline void rtos_usb_endpoint_state_reset(rtos_usb_t *ctx,
                                                 uint32_t endpoint_addr)
{
    (void) ctx;
    XUD_ResetEpStateByAddr(endpoint_addr);
}

/**
 * Stalls a USB endpoint. The stall is cleared automatically when a
 * setup packet is received on the endpoint. Otherwise it can be cleared
 * manually with rtos_usb_endpoint_stall_clear().
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to stall.
 */
static inline void rtos_usb_endpoint_stall_set(rtos_usb_t *ctx,
                                               uint32_t endpoint_addr)
{
    (void) ctx;
    XUD_SetStallByAddr(endpoint_addr);
}

/**
 * Clears the stall condition on USB endpoint.
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to clear the stall on.
 */
static inline void rtos_usb_endpoint_stall_clear(rtos_usb_t *ctx,
                                                 uint32_t endpoint_addr)
{
    (void) ctx;
    XUD_ClearStallByAddr(endpoint_addr);
}

/**
 * Starts the USB driver instance's low level USB I/O thread and enables its interrupts
 * on the requested core. This must only be called by the tile that owns the driver instance.
 * It must be called after starting the RTOS from an RTOS thread.
 *
 * rtos_usb_init() must be called on this USB driver instance prior to calling this.
 *
 * \param ctx                   A pointer to the USB driver instance to start.
 * \param endpoint_count        The number of endpoints that will be used by the application. A single
 *                              endpoint here includes both its IN and OUT endpoints. For example, if the
 *                              application uses EP0_IN, EP0_OUT, EP1_IN, EP2_IN, EP2_OUT, EP3_OUT, then the
 *                              endpoint count specified here should be 4 (endpoint 0 through endpoint 3)
 *                              regardless of the lack of EP1_OUT and EP3_IN. If these two endpoints were used,
 *                              the count would still be 4.\n
 *                              If for whatever reason, the application needs to use a particular endpoint
 *                              number, say only EP6 in addition to EP0, then the count here needs to be 7, even
 *                              though endpoints 1 through 5 are unused. All unused endpoints must be marked as
 *                              disabled in the two endpoint type lists \p endpoint_out_type and \p endpoint_in_type.
 * \param endpoint_out_type     A list of the endpoint types for each output endpoint. Index 0 represents the type
 *                              for EP0_OUT, and so on. See XUD_EpType in lib_xud. If the endpoint is unused, it must
 *                              be set to XUD_EPTYPE_DIS.
 * \param endpoint_in_type      A list of the endpoint types for each input endpoint. Index 0 represents the type
 *                              for EP0_IN, and so on. See XUD_EpType in lib_xud. If the endpoint is unused, it must
 *                              be set to XUD_EPTYPE_DIS.
 * \param speed                 The speed at which the bus should operate. Either XUD_SPEED_FS or XUD_SPEED_HS. See
 *                              XUD_BusSpeed_t in lib_xud.
 * \param power_source          The source of the device's power. Either bus powered (XUD_PWR_BUS) or self powered
 *                              (XUD_PWR_SELF). See XUD_PwrConfig in lib_xud.
 * \param interrupt_core_id     The ID of the core on which to enable the USB interrupts.
 * \param sof_interrupt_core_id The ID of the core on which to enable the SOF interrupt. Set to < 0 to disable
 *                              the SoF interrupt if it is not needed.
 */
void rtos_usb_start(
        rtos_usb_t *ctx,
        size_t endpoint_count,
        XUD_EpType endpoint_out_type[],
        XUD_EpType endpoint_in_type[],
        XUD_BusSpeed_t speed,
        XUD_PwrConfig power_source,
        unsigned interrupt_core_id,
        int sof_interrupt_core_id);

/**
 * Initializes an RTOS USB driver instance. This must only be called by the tile that
 * owns the driver instance. It should be called prior to starting the RTOS, and must
 * be called before any of the core USB driver functions are called with this instance.
 *
 * This will create an RTOS thread that runs lib_xud's main loop. This thread is created with
 * the highest priority and with preemption disabled.
 *
 * \note Due to implementation details of lib_xud, it is only possible to have one
 * USB instance per application. Functionally this is not an issue, as no XCore chips
 * have more than one USB interface.
 *
 * \note If using the Tiny USB stack, then this function should not be called directly
 * by the application. The XCore device port for Tiny USB takes care of calling this, as
 * well as all other USB driver functions.
 *
 * \param ctx               A pointer to the USB driver instance to start.
 * \param io_core_mask      A bitmask representing the cores on which the low level USB I/O thread
 *                          created by the driver is allowed to run. Bit 0 is core 0, bit 1 is core 1,
 *                          etc.
 * \param isr_cb            The callback function for the driver to call when transfers are completed.
 * \param isr_app_data      A pointer to application specific data to pass to the application's ISR
 *                          callback function \p isr_cb.
 */
void rtos_usb_init(
        rtos_usb_t *ctx,
        uint32_t io_core_mask,
        rtos_usb_isr_cb_t isr_cb,
        void *isr_app_data);


/**
 * This function may be called to wait for a transfer on a particular endpoint
 * to complete. This requires that the USB instance was initialized with
 * rtos_usb_simple_init().
 *
 * \param ctx           A pointer to the USB driver instance to use.
 * \param endpoint_addr The address of the endpoint to wait for.
 * \param len           The actual number of bytes transferred. For IN endpoints,
 *                      this will be the same as the length requested by
 *                      rtos_usb_endpoint_transfer_start(). For OUT endpoints, it
 *                      may be less.
 * \param timeout       The maximum amount of time to wait for the transfer to complete
 *                      before returning.
 *
 * \retval XUD_RES_OKAY if the transfer was completed successfully.
 * \retval XUD_RES_RST  if the transfer was not able to complete and the USB bus needs
 *                      to be reset. In this case, the application should reset
 *                      the USB bus.
 * \retval XUD_RES_ERR  if there was an unexpected error transferring the data.
 */
XUD_Result_t rtos_usb_simple_transfer_complete(rtos_usb_t *ctx,
                                               uint32_t endpoint_addr,
                                               size_t *len,
                                               unsigned timeout);

/**
 * Initializes an RTOS USB driver instance. This must only be called by the tile that
 * owns the driver instance. It should be called prior to starting the RTOS, and must
 * be called before any of the core USB driver functions are called with this instance.
 *
 * This initialization function may be used instead of rtos_usb_init() if the application
 * is not using a USB stack. This allows application threads to wait for transfers to complete
 * with the rtos_usb_simple_transfer_complete() function. The application cannot provide its
 * own ISR callback when initialized with this function. This provides a similar programming
 * interface as a traditional bare metal XCore application using lib_xud.
 *
 * This will create an RTOS thread that runs lib_xud's main loop. This thread is created with
 * the highest priority and with preemption disabled.
 *
 * \note Due to implementation details of lib_xud, it is only possible to have one
 * USB instance per application. Functionally this is not an issue, as no XCore chips
 * have more than one USB interface.
 *
 * \param ctx               A pointer to the USB driver instance to start.
 * \param io_core_mask      A bitmask representing the cores on which the low level USB I/O thread
 *                          created by the driver is allowed to run. Bit 0 is core 0, bit 1 is core 1,
 *                          etc.
 *
 */
void rtos_usb_simple_init(
        rtos_usb_t *ctx,
        uint32_t io_core_mask);


/**@}*/

#endif /* RTOS_USB_H_ */
