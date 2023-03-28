
#include <xcore/triggerable.h>
#include "rtos_interrupt.h"
#include "rtos_osal.h"
#include "rtos_usb.h"
#include "rtos_ep0_proxy.h"
#include "xud_xfer_data.h"
#include <string.h>
#include <stdbool.h>

static rtos_osal_queue_t _ep0_proxy_event_queue;
volatile unsigned char sbuffer[120];
static bool waiting_for_setup = false;
extern rtos_usb_t usb_ctx;

void ep0_proxy_init(
    chanend_t chan_ep0_out,
    chanend_t chan_ep0_in,
    chanend_t chan_ep0_proxy,
    chanend_t c_ep0_proxy_xfer_complete)
{
    rtos_usb_t *ctx = &usb_ctx;

    memset(ctx, 0, sizeof(rtos_usb_t));
    // EP0 out and in channels
    ctx->c_ep[0][RTOS_USB_OUT_EP] = chan_ep0_out;
    ctx->c_ep[0][RTOS_USB_IN_EP] = chan_ep0_in;
    ctx->c_ep0_proxy = chan_ep0_proxy;
    ctx->c_ep0_proxy_xfer_complete = c_ep0_proxy_xfer_complete;
    rtos_osal_queue_create(&_ep0_proxy_event_queue, "ep0_proxy_q", 4, sizeof(ep0_proxy_event_t));

    /*chanend_t chan_notify = chanend_alloc();
    chanend_set_dest(chan_notify, chan_notify);
    ep0_proxy_ctx->chan_notify = chan_notify;*/
}

extern volatile uint32_t noEpOut;
extern volatile uint32_t noEpIn;
extern volatile XUD_EpType epTypeTableOut[RTOS_USB_ENDPOINT_COUNT_MAX];
extern volatile XUD_EpType epTypeTableIn[RTOS_USB_ENDPOINT_COUNT_MAX];

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

static void prepare_setup(rtos_usb_t *ctx)
{
    XUD_Result_t res;

//  rtos_printf("preparing for setup packet\n");
    waiting_for_setup = true;
    res = rtos_usb_endpoint_transfer_start(ctx, 0x00, (uint8_t *) &sbuffer[0], 120);

    xassert(res == XUD_RES_OKAY);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(usb_ep0_isr, arg)
{
    rtos_usb_ep_xfer_info_t *ep_xfer_info = arg;
    rtos_usb_t *ctx = ep_xfer_info->usb_ctx;
    const int ep_num = ep_xfer_info->ep_num;
    const int dir = ep_xfer_info->dir;
    size_t xfer_len;
    XUD_Result_t res;
    //printf("In usb_ep0_isr, dir %d\n", dir);

    int is_setup;
    res = ep_transfer_complete(ctx, ep_num, dir, &xfer_len, &is_setup);

    //printintln(3330 | dir);
    triggerable_disable_trigger(ctx->c_ep0_proxy);
    triggerable_disable_trigger(ctx->c_ep[ep_num][dir]);
    uint8_t other_dir = (dir == 0) ? 1 : 0;
    triggerable_disable_trigger(ctx->c_ep[ep_num][other_dir]);

    //printf("usb_ep0_isr: dir = %d, res = %d, xfer_len = %d, is_setup = %d\n", dir, res, xfer_len, is_setup);
    ep_xfer_info->res = (int32_t) res;
    //printintln(ep_xfer_info->res);

    ep0_proxy_event_t event;
    event.event_id = EP0_TRANSFER_COMPLETE;
    event.xfer_complete.ep_num = 0;
    event.xfer_complete.dir = dir;
    event.xfer_complete.result = res;
    event.xfer_complete.is_setup = is_setup;
    event.xfer_complete.len = xfer_len;
    //printintln(1234);
    rtos_osal_status_t status = rtos_osal_queue_send(&_ep0_proxy_event_queue, &event, 0);
    xassert(status == RTOS_OSAL_SUCCESS);

}

DEFINE_RTOS_INTERRUPT_CALLBACK(ep0_proxy_isr, arg)
{
    rtos_usb_t *ctx = arg;
    triggerable_disable_trigger(ctx->c_ep0_proxy);

    //printf("In ep0_proxy_isr\n");
    uint8_t cmd = chan_in_byte(ctx->c_ep0_proxy);
    //uint8_t ep_addr = chan_in_byte(ctx->c_ep0_proxy);
    
    ep0_proxy_event_t event;
    event.event_id = EP0_PROXY_CMD;
    event.ep0_command.cmd = cmd;
    rtos_osal_status_t status = rtos_osal_queue_send(&_ep0_proxy_event_queue, &event, 0);
    //printintln(status);
    xassert(status == RTOS_OSAL_SUCCESS);

}

static void ep_cfg(rtos_usb_t *ctx,
                   int ep_num,
                   int direction)
{
    printf("in my_ep_cfg(): ep_num = %d, direction = %d\n",ep_num, direction);

    ctx->ep_xfer_info[ep_num][direction].dir = direction;
    ctx->ep_xfer_info[ep_num][direction].ep_num = ep_num;
    ctx->ep_xfer_info[ep_num][direction].ep_address = (direction << 7) | ep_num;
    ctx->ep_xfer_info[ep_num][direction].usb_ctx = ctx;
    triggerable_setup_interrupt_callback(ctx->c_ep[ep_num][direction], &ctx->ep_xfer_info[ep_num][direction], RTOS_INTERRUPT_CALLBACK(usb_ep0_isr));
}

static void handle_ep0_command(rtos_usb_t *ctx, uint8_t ep0_cmd)
{
    switch(ep0_cmd)
    {
        case e_reset_ep:
        {
            printf("In e_reset_ep\n");
            ctx->reset_received = 1;
            uint8_t ep_addr = chan_in_byte(ctx->c_ep0_proxy);
            XUD_BusSpeed_t xud_speed;
            xud_speed = rtos_usb_endpoint_reset(ctx, ep_addr);
            prepare_setup(ctx);
            chan_out_byte(ctx->c_ep0_proxy, xud_speed);

            triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_OUT_EP]);
            triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_IN_EP]);
        }                
        break;
        case e_prepare_setup:
        {
            printf("In e_prepare_setup\n");
            prepare_setup(ctx);
            chan_out_byte(ctx->c_ep0_proxy, 0);
        }
        break;
        case e_usb_endpoint_transfer_start:
        {
            printf("In e_usb_endpoint_transfer_start\n");
            uint8_t ep_addr = chan_in_byte(ctx->c_ep0_proxy);
            uint8_t len = chan_in_byte(ctx->c_ep0_proxy);
            if((len > 0) && (endpoint_dir(ep_addr) == RTOS_USB_IN_EP))
            {
                chan_in_buf_byte(ctx->c_ep0_proxy, (uint8_t*)sbuffer, len);
            }
            //printf("ep_addr %d, len = %d\n", ep_addr, len);
            XUD_Result_t res = rtos_usb_endpoint_transfer_start(ctx, (uint32_t)ep_addr, (uint8_t*)sbuffer, len);
            //printf("res = %d\n",res);
            chan_out_byte(ctx->c_ep0_proxy, res);
        }
        break;
        case e_usb_device_address_set:
        {
            printf("In e_usb_device_address_set\n");
            uint32_t dev_addr = chan_in_word(ctx->c_ep0_proxy);
            XUD_Result_t res = rtos_usb_device_address_set(ctx, dev_addr);
            chan_out_byte(ctx->c_ep0_proxy, res);
        }
        break;
    }
    // Enable interrupts
    triggerable_enable_trigger(ctx->c_ep0_proxy);
}

static inline void handle_usb_transfer_complete(rtos_usb_t *ctx, ep0_proxy_event_t *event)
{
    chan_out_byte(ctx->c_ep0_proxy_xfer_complete, event->xfer_complete.dir);
    chan_out_byte(ctx->c_ep0_proxy_xfer_complete, event->xfer_complete.is_setup);
    chan_out_word(ctx->c_ep0_proxy_xfer_complete, event->xfer_complete.len);
    chan_out_word(ctx->c_ep0_proxy_xfer_complete, event->xfer_complete.result);

    if(event->xfer_complete.is_setup)
    {
        // Send setup packet to EP0
        chan_out_buf_byte(ctx->c_ep0_proxy_xfer_complete, (uint8_t*)sbuffer, event->xfer_complete.len); // Will this cause the interrupt on chan_ep0_proxy to trigger
    }
}


void ep0_proxy_task(void *app_data)
{
    unsigned c_ep0_out_interrupt_core_id = 2;
    rtos_usb_t *ctx = (rtos_usb_t*)&usb_ctx;

    noEpOut = chan_in_word(ctx->c_ep0_proxy);
    chan_in_buf_byte(ctx->c_ep0_proxy, (uint8_t*)&epTypeTableOut[0], noEpOut*sizeof(XUD_EpType));
    noEpIn = chan_in_word(ctx->c_ep0_proxy);
    chan_in_buf_byte(ctx->c_ep0_proxy, (uint8_t*)&epTypeTableIn[0], noEpIn*sizeof(XUD_EpType));

    
    /* Ensure that all USB interrupts are enabled on the requested core */
    uint32_t core_exclude_map;
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << c_ep0_out_interrupt_core_id));

    ep_cfg(ctx, 0, RTOS_USB_OUT_EP);
    ep_cfg(ctx, 0, RTOS_USB_IN_EP);

    triggerable_setup_interrupt_callback(ctx->c_ep0_proxy, ctx, RTOS_INTERRUPT_CALLBACK(ep0_proxy_isr));
    
    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);
    
    // Signal to _XUD_Main to start USB
    chan_out_byte(ctx->c_ep[0][RTOS_USB_OUT_EP], 1);
    
    ctx->ep[0][RTOS_USB_OUT_EP] = XUD_InitEp(ctx->c_ep[0][RTOS_USB_OUT_EP]);
    ctx->ep[0][RTOS_USB_IN_EP] = XUD_InitEp(ctx->c_ep[0][RTOS_USB_IN_EP]);

    printf("EP0 endpoints initialized\n");

    triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_OUT_EP]);
    triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_IN_EP]);

    // Enable ISR
    printf("In ep0_proxy_task(), 0x%x, 0x%x\n", epTypeTableOut[0], epTypeTableIn[0]);
    
    ep0_proxy_event_t event;
    while(1)
    {   
        rtos_osal_queue_receive(&_ep0_proxy_event_queue, &event, RTOS_OSAL_WAIT_FOREVER);
        // Send this to EP0 and wait for commands from EP0
        if(event.event_id == EP0_TRANSFER_COMPLETE)
        {
            handle_usb_transfer_complete(ctx, &event);

            // TODO If an out data xfer is  completed the data needs to be sent to EP0. This is currently not handled.

            //rtos_printf("Enable c_ep0_proxy ISR on tile 0 on chanend %ld\n", ctx->c_ep0_proxy);
            triggerable_enable_trigger(ctx->c_ep0_proxy); // Enable proxy ISR and wait for offtile EP0 to communicate with us

            if(event.xfer_complete.result == XUD_RES_OKAY)
            {
                // If everything was okay, we might not hear from ep0 at all, so enable the EP interrupts
                triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_OUT_EP]);
                triggerable_enable_trigger(ctx->c_ep[0][RTOS_USB_IN_EP]);

            }
        }
        else if(event.event_id == EP0_PROXY_CMD)
        {
            handle_ep0_command(ctx, event.ep0_command.cmd);
        }
    }


    return;
}

void ep0_proxy_start(unsigned priority)
{
    xTaskCreate((TaskFunction_t) ep0_proxy_task,
        "ep0_proxy_task",
        RTOS_THREAD_STACK_SIZE(ep0_proxy_task),
        NULL,
        priority,
        NULL);
}
