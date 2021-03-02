// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef RTOS_USB_H_
#define RTOS_USB_H_

/**
 * \defgroup rtos_usb_driver
 *
 * The public API for using the RTOS USB driver.
 * @{
 */

#include <xcore/channel.h>
#include <xud.h>
#include <xud_device.h>

#include "rtos/drivers/osal/api/rtos_osal.h"
#include "rtos/drivers/rpc/api/rtos_driver_rpc.h"

/**
 * The maximum number of USB endpoint numbers supported by the RTOS USB driver.
 */
#define RTOS_USB_ENDPOINT_COUNT_MAX 12


/*
 * These are used to index into the first index of many of the
 * RTOS USB driver's endpoint arrays.
 * @{
 */
#define RTOS_USB_OUT_EP 0
#define RTOS_USB_IN_EP  1
/**@}*/

/**
 * This attribute must be specified on the RTOS USB interrupt callback function
 * provided by the application.
 */
#define RTOS_USB_ISR_CALLBACK_ATTR __attribute__((fptrgroup("rtos_usb_isr_cb_fptr_grp")))

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
 * \param is_setup      True if this is a setup packet. False otherwise.
 * \param res           The result of the transfer. See XUD_Result_t.
 */
typedef void (*rtos_usb_isr_cb_t)(rtos_usb_t *ctx, void *app_data, uint32_t ep_address, size_t xfer_len, int is_setup, XUD_Result_t res);

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

    XUD_EpType endpoint_out_type[RTOS_USB_ENDPOINT_COUNT_MAX];
    XUD_EpType endpoint_in_type[RTOS_USB_ENDPOINT_COUNT_MAX];
    XUD_PwrConfig power_source;
    XUD_BusSpeed_t speed;

    chanend_t c_ep[RTOS_USB_ENDPOINT_COUNT_MAX][2];
    XUD_ep ep[RTOS_USB_ENDPOINT_COUNT_MAX][2];
    rtos_osal_event_group_t event_group;
    RTOS_USB_ISR_CALLBACK_ATTR rtos_usb_isr_cb_t isr_cb;
    void *isr_app_data;
    rtos_usb_ep_xfer_info_t ep_xfer_info[RTOS_USB_ENDPOINT_COUNT_MAX][2];
};

/**
 * Checks to see if a particular endpoint is ready to use. This only needs
 * to be checked once after starting the USB driver instance.
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
 * Checks to see if all endpoints are ready to use. This only needs
 * to be checked once after starting the USB driver instance.
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
 * This function may be called to wait for a transfer on a particular endpoint
 * to complete.
 *
 * \note
 * To use this function, the application must provide rtos_usb_start() with the
 * usb_simple_isr_cb() callback function. If the application instead chooses to
 * provide its own ISR callback, then this function should not be used.
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
XUD_Result_t rtos_usb_endpoint_transfer_complete(rtos_usb_t *ctx,
                                                 uint32_t endpoint_addr,
                                                 size_t *len,
                                                 unsigned timeout);

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
 * This callback may be used by the application so that it may wait
 * for endpoint transfers to complete with rtos_usb_endpoint_transfer_complete().
 * This provides a similar usage model to bare metal applications that use lib_xud
 * directly.
 *
 * To use this, it must be provided as the isr_cb parameter to rtos_usb_start().
 */
void usb_simple_isr_cb(rtos_usb_t *ctx,
                       void *app_data,
                       uint32_t ep_address,
                       size_t xfer_len,
                       int is_setup,
                       XUD_Result_t res);

/**
 * Starts an RTOS USB driver instance. This must only be called by the tile that
 * owns the driver instance. It must be called after starting the RTOS, and must
 * be called before any of the core GPIO driver functions are called with this instance.
 *
 * This will start an RTOS thread that runs lib_xud's main loop. The driver will ensure
 * that this thread is not started on core 0 so that the system tick interrupt does not
 * interfere with it. It will also disable preemption on this thread, such that once it
 * is running, no other tasks will be able to interfere with it, even if they have a
 * higher priority.
 *
 * \note Due to implementation details of lib_xud, it is only possible to have one
 * USB instance per application. Functionally this is not an issue, as no XCore chips
 * have more than one USB interface. For the same reason, there is no need for an init
 * function, as all I/O initialization takes place by lib_xud after the driver instance
 * is started.
 *
 * \param ctx               A pointer to the USB driver instance to start.
 * \param isr_cb            The callback function for the driver to call when transfers are completed.
 * \param isr_app_data      A pointer to application specific data to pass to the application's ISR
 *                          callback function \p isr_cb.
 * \param endpoint_count    The number of endpoints that will be used by the application. A single
 *                          endpoint here includes both its IN and OUT endpoints. For example, if the
 *                          application uses EP0_IN, EP0_OUT, EP1_IN, EP2_IN, EP2_OUT, EP3_OUT, then the
 *                          endpoint count specified here should be 4 (endpoint 0 through endpoint 3)
 *                          regardless of the lack of EP1_OUT and EP3_IN. If these two endpoints were used,
 *                          the count would still be 4.
 *                          \note if for whatever reason, the application needs to use a particular endpoint
 *                          number, say only EP6 in addition to EP0, then the count here needs to be 7, even
 *                          though endpoints 1 through 5 are unused. All unused endpoints must be marked as
 *                          unused in the two endpoint type lists \p endpoint_out_type and \p endpoint_in_type.
 * \param endpoint_out_type A list of the endpoint types for each output endpoint. Index 0 represents the type
 *                          for EP0_OUT, and so on. See XUD_EpType in lib_xud. If the endpoint is unused, it must
 *                          be set to XUD_EPTYPE_DIS.
 * \param endpoint_in_type  A list of the endpoint types for each input endpoint. Index 0 represents the type
 *                          for EP0_IN, and so on. See XUD_EpType in lib_xud. If the endpoint is unused, it must
 *                          be set to XUD_EPTYPE_DIS.
 * \param speed             The speed at which the bus should operate. Either XUD_SPEED_FS or XUD_SPEED_HS. See
 *                          XUD_BusSpeed_t in lib_xud.
 * \param XUD_PwrConfig     The source of the device's power. Either bus powered (XUD_PWR_BUS) or self powered
 *                          (XUD_PWR_SELF). See XUD_PwrConfig in lib_xud.
 * \param priority          The priority to assign to the task that runs lib_xud's main loop.
 *
 */
void rtos_usb_start(
        rtos_usb_t *ctx,
        rtos_usb_isr_cb_t isr_cb,
        void *isr_app_data,
        size_t endpoint_count,
        XUD_EpType endpoint_out_type[],
        XUD_EpType endpoint_in_type[],
        XUD_BusSpeed_t speed,
        XUD_PwrConfig power_source,
        unsigned priority);

/**@}*/

#endif /* RTOS_USB_H_ */
