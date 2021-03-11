// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_USB

#include <string.h>
#include <xcore/triggerable.h>
#include "rtos_interrupt.h"
#include "rtos_usb.h"
#include "xud_xfer_data.h"

int XUD_Main(chanend_t c_epOut[],
             int noEpOut,
             chanend_t c_epIn[],
             int noEpIn,
             chanend_t c_sof,
             XUD_EpType epTypeTableOut[],
             XUD_EpType epTypeTableIn[],
#if !XUD_DEV_XS3
             unsigned a, unsigned b, unsigned c,
#endif
             XUD_BusSpeed_t desiredSpeed,
             XUD_PwrConfig pwrConfig);

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));
#define CLRSR(c) asm volatile("clrsr %0" : : "n"(c));

static void usb_xud_thread(rtos_usb_t *ctx)
{
    /* Ensure the USB thread is never preempted */
    rtos_osal_thread_preemption_disable(NULL);
    /* And exclude it from core 0 where the system tick interrupt runs */
    rtos_osal_thread_core_exclusion_set(NULL, (1 << 0));

    rtos_interrupt_mask_all();
    /* TODO:
     * Disable the interrupt on the FreeRTOS intercore chanend.
     * Disabling preemption above should ensure that under normal
     * conditions it is not interrupted, but there are some cases
     * where it might be. If that were to happen, I believe it would
     * crash since KEDI will be off.
     *
     * IMPORTANT:
     * The application currently needs to take care that it does not
     * have any interrupts enabled on any cores other than core 0
     * before this point. If this thread ends up on a core that has
     * any interrupts enabled, this will result in some very strange
     * crashes.
     * TODO: This needs to be handled better.
     */

    CLRSR(XS1_SR_KEDI_MASK);

    rtos_printf("Starting XUD_Main() with %d endpoints\n", ctx->endpoint_count);

    XUD_Main(ctx->c_ep_out_xud,
             ctx->endpoint_count,
             ctx->c_ep_in_xud,
             ctx->endpoint_count,
             0/*ctx->c_sof_xud*/,
             ctx->endpoint_out_type,
             ctx->endpoint_in_type,
#if !XUD_DEV_XS3
             0, 0, -1,
#endif
             ctx->speed,
             ctx->power_source);

    SETSR(XS1_SR_KEDI_MASK);

    /* TODO:
     * Re-enable the interrupt on the intercore chanend.
     */

    rtos_interrupt_unmask_all();
    vTaskDelete(NULL);
}

static inline unsigned ep_event_flag(const int ep_num,
                                     const int dir)
{
    return 1 << (ep_num + (dir ? RTOS_USB_ENDPOINT_COUNT_MAX : 0));
}

static XUD_Result_t ep_transfer_complete(rtos_usb_t *ctx,
                                         const int ep_num,
                                         const int dir,
                                         size_t *len,
                                         int *is_setup)
{
    XUD_Result_t res = XUD_RES_ERR;

    xassert(ep_num < RTOS_USB_ENDPOINT_COUNT_MAX);

    if (dir == RTOS_USB_IN_EP) {
        res = xud_data_set_finish(ctx->c_ep[ep_num][dir], ctx->ep[ep_num][dir]);
        *is_setup = 0;
        *len = ctx->ep_xfer_info[ep_num][dir].len;
    } else {
        res = xud_data_get_check(ctx->c_ep[ep_num][dir], ctx->ep[ep_num][dir], len, is_setup);

        if (*len > ctx->ep_xfer_info[ep_num][dir].len) {
            rtos_printf("Length of %d bytes transferred on ep %d direction %d. Should have been <= %d\n", *len, ep_num, dir, ctx->ep_xfer_info[ep_num][dir].len);
        }

        xassert(*len <= ctx->ep_xfer_info[ep_num][dir].len);
        ctx->ep_xfer_info[ep_num][dir].len = *len;

        if (res == XUD_RES_OKAY) {
            if (*is_setup) {
                res = xud_setup_data_get_finish(ctx->ep[ep_num][dir]);
                if (res == XUD_RES_ERR) {
                    rtos_printf("USB XFER ERROR from xud_setup_data_get_finish()!\n");
                }
            } else {
                res = xud_data_get_finish(ctx->ep[ep_num][dir]);
                if (res == XUD_RES_ERR) {
                    rtos_printf("USB XFER ERROR from xud_data_get_finish()!\n");
                }
            }
        } else if (res == XUD_RES_ERR) {
            rtos_printf("USB XFER ERROR from xud_data_get_check()!\n");
        }
    }

    return res;
}

DEFINE_RTOS_INTERRUPT_CALLBACK(usb_isr, arg)
{
    rtos_usb_ep_xfer_info_t *ep_xfer_info = arg;
    rtos_usb_t *ctx = ep_xfer_info->usb_ctx;
    const int ep_num = ep_xfer_info->ep_num;
    const int dir = ep_xfer_info->dir;
    size_t xfer_len;
    XUD_Result_t res;

    //rtos_printf("Disable trigger on %d %d\n", ep_num, dir);
//    triggerable_disable_trigger(ctx->c_ep[ep_num][dir]);

    if (ctx->ep[ep_num][dir] != 0) {
        int is_setup;
        res = ep_transfer_complete(ctx, ep_num, dir, &xfer_len, &is_setup);
        ep_xfer_info->res = (int32_t) res;

        if (ctx->isr_cb != NULL) {
            ctx->isr_cb(ctx, ctx->isr_app_data, ep_xfer_info->ep_address, xfer_len, is_setup, res);
        }
    } else {
        ctx->ep[ep_num][dir] = XUD_InitEp(ctx->c_ep[ep_num][dir]);
        rtos_printf("EP %d %d initialized\n", ep_num, dir);
        rtos_osal_event_group_set_bits(&ctx->event_group, ep_event_flag(ep_num, dir));
    }
}

static int endpoint_wait(rtos_usb_t *ctx,
                          const uint32_t ep_flags,
                          unsigned timeout)
{
    rtos_osal_status_t status;
    uint32_t flags;

    status = rtos_osal_event_group_get_bits(
            &ctx->event_group,
            ep_flags,
            RTOS_OSAL_AND_CLEAR,
            &flags,
            timeout);

    if (status == RTOS_OSAL_SUCCESS) {
        return 0;
    } else {
        return -1;
    }
}

static inline int endpoint_num(uint32_t endpoint_addr)
{
    return endpoint_addr & 0xF;
}

static inline int endpoint_dir(uint32_t endpoint_addr)
{
    return (endpoint_addr >> 7) & 1;
}

XUD_Result_t rtos_usb_endpoint_ready(rtos_usb_t *ctx,
                                     uint32_t endpoint_addr,
                                     unsigned timeout)
{
    const int ep_num = endpoint_num(endpoint_addr);
    const int dir = endpoint_dir(endpoint_addr);

    if (endpoint_wait(ctx, ep_event_flag(ep_num, dir), timeout) == 0) {
        return XUD_RES_OKAY;
    } else {
        return XUD_RES_ERR;
    }
}

XUD_Result_t rtos_usb_all_endpoints_ready(rtos_usb_t *ctx,
                                          unsigned timeout)
{
    int i;
    uint32_t endpoint_flags = 0;

    for (i = 0; i < ctx->endpoint_count; i++) {
        if (ctx->c_ep[i][RTOS_USB_OUT_EP] != 0) {
            endpoint_flags |= ep_event_flag(i, RTOS_USB_OUT_EP);
        }
        if (ctx->c_ep[i][RTOS_USB_IN_EP] != 0) {
            endpoint_flags |= ep_event_flag(i, RTOS_USB_IN_EP);
        }
    }

    if (endpoint_wait(ctx, endpoint_flags, timeout) == 0) {
        return XUD_RES_OKAY;
    } else {
        return XUD_RES_ERR;
    }
}

XUD_Result_t rtos_usb_endpoint_transfer_start(rtos_usb_t *ctx,
                                              uint32_t endpoint_addr,
                                              uint8_t *buffer,
                                              size_t len)
{
    XUD_Result_t res;
    const int ep_num = endpoint_num(endpoint_addr);
    const int dir = endpoint_dir(endpoint_addr);

    xassert(ep_num < RTOS_USB_ENDPOINT_COUNT_MAX);

    ctx->ep_xfer_info[ep_num][dir].len = len;

    if (dir == RTOS_USB_IN_EP) {
        res = xud_data_set_start(ctx->ep[ep_num][dir], buffer, len);
    } else {
        res = xud_data_get_start(ctx->ep[ep_num][dir], buffer);
    }

    if (res == XUD_RES_OKAY) {
        //rtos_printf("Enable trigger on %d %d\n", ep_num, dir);
//        triggerable_enable_trigger(ctx->c_ep[ep_num][dir]);
    }

    return res;
}

RTOS_USB_ISR_CALLBACK_ATTR
void usb_simple_isr_cb(rtos_usb_t *ctx,
                       void *app_data,
                       uint32_t ep_address,
                       size_t xfer_len,
                       int is_setup,
                       XUD_Result_t res)
{
    (void) app_data;
    (void) xfer_len;
    (void) res;
    (void) is_setup;

    rtos_osal_event_group_set_bits(&ctx->event_group,
                                   ep_event_flag(endpoint_num(ep_address),
                                                 endpoint_dir(ep_address)));
}

/*
 * Assumes that usb_simple_isr_cb() is used as (or called by) the ISR callback.
 */
XUD_Result_t rtos_usb_endpoint_transfer_complete(rtos_usb_t *ctx,
                                                 uint32_t endpoint_addr,
                                                 size_t *len,
                                                 unsigned timeout)
{
    const int ep_num = endpoint_num(endpoint_addr);
    const int dir = endpoint_dir(endpoint_addr);

    if (endpoint_wait(ctx, ep_event_flag(ep_num, dir), timeout) == 0) {
        if (len != NULL) {
            *len = ctx->ep_xfer_info[ep_num][dir].len;
        }
        return ctx->ep_xfer_info[ep_num][dir].res;
    } else {
        return XUD_RES_ERR;
    }
}

XUD_BusSpeed_t rtos_usb_endpoint_reset(rtos_usb_t *ctx,
                                       uint32_t endpoint_addr)
{
    uint8_t const epnum = endpoint_num(endpoint_addr);
    uint8_t dir = endpoint_dir(endpoint_addr);

    XUD_ep one = ctx->ep[epnum][dir];
    XUD_ep *two = NULL;

    dir = dir ? 0 : 1;

    if (ctx->ep[epnum][dir] != 0) {
        two = &ctx->ep[epnum][dir];
    }

    if (one == 0) {
        xassert(two != NULL);
        one = *two;
        two = NULL;
    }

    return XUD_ResetEndpoint(one, two);
}

void rtos_usb_start(
        rtos_usb_t *ctx,
        rtos_usb_isr_cb_t isr_cb,
        void *isr_app_data,
        size_t endpoint_count,
        XUD_EpType endpoint_out_type[],
        XUD_EpType endpoint_in_type[],
        XUD_BusSpeed_t speed,
        XUD_PwrConfig power_source,
        unsigned priority)
{
    int i;
    channel_t tmp_chan;
    uint32_t core_exclude_map;

    xassert(endpoint_count <= RTOS_USB_ENDPOINT_COUNT_MAX);

    memset(ctx, 0, sizeof(rtos_usb_t));

    ctx->isr_cb = isr_cb;
    ctx->isr_app_data = isr_app_data;

    ctx->endpoint_count = endpoint_count;

    ctx->power_source = power_source;
    ctx->speed = speed;

    rtos_osal_event_group_create(&ctx->event_group, "usb_ev_grp");

    /*
     * Force the channel interrupts to be enabled on core 0,
     * where XUD_Main() is excluded from. If XUD_Main() ended up
     * running on the core where these interrupts are enabled, this
     * would be a problem.
     */
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << 0));

    for (i = 0; i < endpoint_count; i++) {

        ctx->endpoint_out_type[i] = endpoint_out_type[i];
        if (endpoint_out_type[i] != XUD_EPTYPE_DIS) {

            tmp_chan = chan_alloc();
            ctx->c_ep_out_xud[i] = tmp_chan.end_a;
            ctx->c_ep[i][RTOS_USB_OUT_EP] = tmp_chan.end_b;

            ctx->ep_xfer_info[i][RTOS_USB_OUT_EP].dir = RTOS_USB_OUT_EP;
            ctx->ep_xfer_info[i][RTOS_USB_OUT_EP].ep_num = i;
            ctx->ep_xfer_info[i][RTOS_USB_OUT_EP].ep_address = i;
            ctx->ep_xfer_info[i][RTOS_USB_OUT_EP].usb_ctx = ctx;
            triggerable_setup_interrupt_callback(ctx->c_ep[i][RTOS_USB_OUT_EP], &ctx->ep_xfer_info[i][RTOS_USB_OUT_EP], RTOS_INTERRUPT_CALLBACK(usb_isr));
            triggerable_enable_trigger(ctx->c_ep[i][RTOS_USB_OUT_EP]);
        }

        ctx->endpoint_in_type[i] = endpoint_in_type[i];
        if (endpoint_in_type[i] != XUD_EPTYPE_DIS) {

            tmp_chan = chan_alloc();
            ctx->c_ep_in_xud[i] = tmp_chan.end_a;
            ctx->c_ep[i][RTOS_USB_IN_EP] = tmp_chan.end_b;

            ctx->ep_xfer_info[i][RTOS_USB_IN_EP].dir = RTOS_USB_IN_EP;
            ctx->ep_xfer_info[i][RTOS_USB_IN_EP].ep_num = i;
            ctx->ep_xfer_info[i][RTOS_USB_IN_EP].ep_address = 0x80 | i;
            ctx->ep_xfer_info[i][RTOS_USB_IN_EP].usb_ctx = ctx;
            triggerable_setup_interrupt_callback(ctx->c_ep[i][RTOS_USB_IN_EP], &ctx->ep_xfer_info[i][RTOS_USB_IN_EP], RTOS_INTERRUPT_CALLBACK(usb_isr));
            triggerable_enable_trigger(ctx->c_ep[i][RTOS_USB_IN_EP]);
        }
    }

    tmp_chan = chan_alloc();
    ctx->c_sof_xud = tmp_chan.end_a;
    ctx->c_sof = tmp_chan.end_b;

    /* Restore the core exclusion map for this thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    rtos_osal_thread_create(
            NULL,
            "usb_xud_thread",
            (rtos_osal_entry_function_t) usb_xud_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(usb_xud_thread),
            priority);
}
