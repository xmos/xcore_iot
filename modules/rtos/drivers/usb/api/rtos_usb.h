// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef RTOS_USB_H_
#define RTOS_USB_H_

#include <xcore/channel.h>
#include <xud.h>
#include <xud_device.h>

#include "rtos/drivers/osal/api/rtos_osal.h"
#include "rtos/drivers/rpc/api/rtos_driver_rpc.h"

#define RTOS_USB_ENDPOINT_COUNT_MAX 12

#define RTOS_USB_OUT_EP 0
#define RTOS_USB_IN_EP  1

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
 * \param res           The result of the transfer. See XUD_Result_t.
 */
typedef void (*rtos_usb_isr_cb_t)(rtos_usb_t *ctx, void *app_data, uint32_t ep_address, size_t xfer_len, XUD_Result_t res);

typedef struct {
    rtos_usb_t *usb_ctx;
    size_t len;
    uint8_t ep_address;
    uint8_t dir;
    uint8_t ep_num;
    int8_t res;
} rtos_usb_ep_xfer_info_t;

struct rtos_usb_struct {
    size_t endpoint_count;
    size_t endpoint_out_count;
    size_t endpoint_in_count;
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

int rtos_usb_endpoint_ready(rtos_usb_t *ctx,
                            uint32_t endpoint_addr,
                            unsigned timeout);

XUD_Result_t rtos_usb_all_endpoints_ready(rtos_usb_t *ctx,
                                          unsigned timeout);

XUD_Result_t rtos_usb_endpoint_transfer_start(rtos_usb_t *ctx,
                                              uint32_t endpoint_addr,
                                              uint8_t *buffer,
                                              size_t len);

XUD_Result_t rtos_usb_endpoint_transfer_complete(rtos_usb_t *ctx,
                                                 uint32_t endpoint_addr,
                                                 size_t *len,
                                                 unsigned timeout);

void usb_simple_isr_cb(rtos_usb_t *ctx,
                       void *app_data,
                       uint32_t ep_address,
                       size_t xfer_len,
                       XUD_Result_t res);

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

#endif /* RTOS_USB_H_ */
