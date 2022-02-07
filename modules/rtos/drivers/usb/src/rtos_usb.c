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
    XUD_EpType endpoint_out_type[RTOS_USB_ENDPOINT_COUNT_MAX];
    XUD_EpType endpoint_in_type[RTOS_USB_ENDPOINT_COUNT_MAX];

    /*
     * XUD_Main() appears to require that interrupts be initially disabled.
     */
    rtos_interrupt_mask_all();

    /*
     * XUD_Main() itself uses interrupts, and does re-enable them. However,
     * it assumes that KEDI is not set, therefore it is cleared here.
     */
    CLRSR(XS1_SR_KEDI_MASK);

    (void) s_chan_in_byte(ctx->c_sof_xud);

    rtos_printf("Starting XUD_Main() on core %d with %d endpoints\n", rtos_core_id_get(), ctx->endpoint_count);

    memcpy(endpoint_out_type, ctx->endpoint_out_type, sizeof(endpoint_out_type));
    memcpy(endpoint_in_type, ctx->endpoint_in_type, sizeof(endpoint_in_type));

    XUD_Main(ctx->c_ep_out_xud,
             ctx->endpoint_count,
             ctx->c_ep_in_xud,
             ctx->endpoint_count,
             ctx->sof_interrupt_enabled ? ctx->c_sof_xud : 0,
             endpoint_out_type,
             endpoint_in_type,
#if !XUD_DEV_XS3
             0, 0, -1,
#endif
             ctx->speed,
             ctx->power_source);

    SETSR(XS1_SR_KEDI_MASK);
    rtos_interrupt_unmask_all();

    vTaskDelete(NULL);
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
        res = xud_data_get_check(ctx->c_ep[ep_num][dir], len, is_setup);

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

    if (ctx->ep[ep_num][dir] != 0) {
        int is_setup;
        res = ep_transfer_complete(ctx, ep_num, dir, &xfer_len, &is_setup);
        ep_xfer_info->res = (int32_t) res;

        if (res == XUD_RES_RST) {
            ctx->reset_received = 1;
        }

        if (ctx->isr_cb != NULL) {
            ctx->isr_cb(ctx, ctx->isr_app_data, ep_xfer_info->ep_address, xfer_len, is_setup ? rtos_usb_setup_packet : rtos_usb_data_packet, res);
        }
    } else {
        ctx->ep[ep_num][dir] = XUD_InitEp(ctx->c_ep[ep_num][dir]);
        rtos_printf("EP %d %d initialized\n", ep_num, dir);
    }
}

DEFINE_RTOS_INTERRUPT_CALLBACK(usb_sof_isr, arg)
{
    rtos_usb_t *ctx = arg;

    (void) s_chan_in_word(ctx->c_sof);

    if (ctx->isr_cb != NULL) {
        ctx->isr_cb(ctx, ctx->isr_app_data, 0, 0, rtos_usb_sof_packet, XUD_RES_OKAY);
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
    rtos_osal_tick_t start_time;

    start_time = rtos_osal_tick_get();
    while (ctx->ep[ep_num][dir] == 0 && rtos_osal_tick_get() - start_time < timeout) {
        rtos_osal_delay(1);
    }

    if (ctx->ep[ep_num][dir] != 0) {
        return XUD_RES_OKAY;
    } else {
        return XUD_RES_ERR;
    }
}

XUD_Result_t rtos_usb_all_endpoints_ready(rtos_usb_t *ctx,
                                          unsigned timeout)
{
    rtos_osal_tick_t start_time;

    start_time = rtos_osal_tick_get();
    while (!ctx->reset_received && rtos_osal_tick_get() - start_time < timeout) {
        rtos_osal_delay(1);
    }

    if (ctx->reset_received) {
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

    if (!ctx->reset_received) {
        return XUD_RES_ERR;
    }

    ctx->ep_xfer_info[ep_num][dir].len = len;

    if (dir == RTOS_USB_IN_EP) {
        res = xud_data_set_start(ctx->ep[ep_num][dir], buffer, len);
    } else {
        res = xud_data_get_start(ctx->ep[ep_num][dir], buffer);
    }

    return res;
}

XUD_BusSpeed_t rtos_usb_endpoint_reset(rtos_usb_t *ctx,
                                       uint32_t endpoint_addr)
{
    uint8_t const epnum = endpoint_num(endpoint_addr);
    uint8_t dir = endpoint_dir(endpoint_addr);

    XUD_ep one = ctx->ep[epnum][dir];
    XUD_ep *two = NULL;

    xassert(ctx->reset_received);

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

static void ep_cfg(rtos_usb_t *ctx,
                   int ep_num,
                   int direction)
{
    channel_t tmp_chan = chan_alloc();

    xassert(tmp_chan.end_a != 0);
    if (direction == RTOS_USB_OUT_EP) {
        ctx->c_ep_out_xud[ep_num] = tmp_chan.end_a;
    } else {
        ctx->c_ep_in_xud[ep_num] = tmp_chan.end_a;
    }
    ctx->c_ep[ep_num][direction] = tmp_chan.end_b;

    ctx->ep_xfer_info[ep_num][direction].dir = direction;
    ctx->ep_xfer_info[ep_num][direction].ep_num = ep_num;
    ctx->ep_xfer_info[ep_num][direction].ep_address = (direction << 7) | ep_num;
    ctx->ep_xfer_info[ep_num][direction].usb_ctx = ctx;
    triggerable_setup_interrupt_callback(ctx->c_ep[ep_num][direction], &ctx->ep_xfer_info[ep_num][direction], RTOS_INTERRUPT_CALLBACK(usb_isr));
    triggerable_enable_trigger(ctx->c_ep[ep_num][direction]);
}

void rtos_usb_start(
        rtos_usb_t *ctx,
        size_t endpoint_count,
        XUD_EpType endpoint_out_type[],
        XUD_EpType endpoint_in_type[],
        XUD_BusSpeed_t speed,
        XUD_PwrConfig power_source,
        unsigned interrupt_core_id,
        int sof_interrupt_core_id)
{
    int i;
    uint32_t core_exclude_map;

    ctx->power_source = power_source;
    ctx->speed = speed;

    xassert(endpoint_count > 0 && endpoint_count <= RTOS_USB_ENDPOINT_COUNT_MAX);
    ctx->endpoint_count = endpoint_count;

    /* Ensure that all USB interrupts are enabled on the requested core */
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);

    if (sof_interrupt_core_id >= 0) {
        ctx->sof_interrupt_enabled = 1;
        rtos_osal_thread_core_exclusion_set(NULL, ~(1 << sof_interrupt_core_id));
        triggerable_setup_interrupt_callback(ctx->c_sof, ctx, RTOS_INTERRUPT_CALLBACK(usb_sof_isr));
        triggerable_enable_trigger(ctx->c_sof);
    }

    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    for (i = 0; i < endpoint_count; i++) {

        ctx->endpoint_out_type[i] = endpoint_out_type[i];
        ctx->endpoint_in_type[i] = endpoint_in_type[i];

        if (endpoint_out_type[i] != XUD_EPTYPE_DIS) {
            ep_cfg(ctx, i, RTOS_USB_OUT_EP);
        }
        if (endpoint_in_type[i] != XUD_EPTYPE_DIS) {
            ep_cfg(ctx, i, RTOS_USB_IN_EP);
        }
    }

    /* Tells the I/O thread to enter XUD_Main() */
    s_chan_out_byte(ctx->c_sof, 0);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);
}

void rtos_usb_init(
        rtos_usb_t *ctx,
        uint32_t io_core_mask,
        rtos_usb_isr_cb_t isr_cb,
        void *isr_app_data)
{
    channel_t tmp_chan;

    memset(ctx, 0, sizeof(rtos_usb_t));

    ctx->isr_cb = isr_cb;
    ctx->isr_app_data = isr_app_data;

    tmp_chan = chan_alloc();
    xassert(tmp_chan.end_a != 0);
    ctx->c_sof_xud = tmp_chan.end_a;
    ctx->c_sof = tmp_chan.end_b;

    rtos_osal_thread_create(
            &ctx->hil_thread,
            "usb_hil_thread",
            (rtos_osal_entry_function_t) usb_xud_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(usb_xud_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the I2C thread is never preempted */
    rtos_osal_thread_preemption_disable(&ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&ctx->hil_thread, ~io_core_mask);
}





static inline unsigned ep_event_flag(const int ep_num,
                                     const int dir)
{
    return 1 << (ep_num + (dir ? RTOS_USB_ENDPOINT_COUNT_MAX : 0));
}

RTOS_USB_ISR_CALLBACK_ATTR
static void usb_simple_isr_cb(rtos_usb_t *ctx,
                              void *app_data,
                              uint32_t ep_address,
                              size_t xfer_len,
                              rtos_usb_packet_type_t packet_type,
                              XUD_Result_t res)

{
    rtos_osal_event_group_t *event_group = app_data;
    (void) xfer_len;
    (void) res;

    if (packet_type == rtos_usb_data_packet || packet_type == rtos_usb_setup_packet) {
        rtos_osal_event_group_set_bits(event_group,
                                       ep_event_flag(endpoint_num(ep_address),
                                                     endpoint_dir(ep_address)));
    }
}

static int endpoint_wait(rtos_usb_t *ctx,
                         const uint32_t ep_flags,
                         unsigned timeout)
{
    rtos_osal_status_t status;
    uint32_t flags;
    rtos_osal_event_group_t *event_group = ctx->isr_app_data;

    status = rtos_osal_event_group_get_bits(
            event_group,
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

XUD_Result_t rtos_usb_simple_transfer_complete(rtos_usb_t *ctx,
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


void rtos_usb_simple_init(
        rtos_usb_t *ctx,
        uint32_t io_core_mask)
{
    static rtos_osal_event_group_t event_group;

    rtos_osal_event_group_create(&event_group, "usb_ev_grp");

    rtos_usb_init(
            ctx,
            io_core_mask,
            usb_simple_isr_cb,
            &event_group);
}
