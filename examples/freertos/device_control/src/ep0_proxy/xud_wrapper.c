/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>
#include "xcore/parallel.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos_printf.h"
#include "usb_support.h"

/* App headers */
#include "app_control/app_control.h"
#include "device_control.h"
#include "device_control_i2c.h"
#include "device_control_usb.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#include <xud.h>
#include <xud_device.h>

#include <stdio.h>
#include "rtos_usb.h"

extern volatile uint32_t noEpOut;
extern volatile uint32_t noEpIn;
extern volatile XUD_EpType epTypeTableOut[RTOS_USB_ENDPOINT_COUNT_MAX];
extern volatile XUD_EpType epTypeTableIn[RTOS_USB_ENDPOINT_COUNT_MAX];

void _XUD_Main(chanend_t c_ep0_Out, chanend_t c_ep0_In, chanend_t c_sof, XUD_BusSpeed_t desiredSpeed, XUD_PwrConfig pwrConfig)
{
    // The order in which things happen
    // 1. offtile ep0 starts and finishes registering servicers on the device_control ctx it hosts.
    // 2. It indicates to ep0 proxy over chan_ep0_proxy that ep0 init is done.
    // 3. ep0 proxy indicates to XUD_Main over chan_ep0_out that XUD_Main can be started.
    printf("Calling XUD_Main()\n");
    chan_in_byte(c_ep0_Out); // EP0 proxy indicates on this channel that it's safe to start XUD_Main.
    
    chanend_t c_ep_out[RTOS_USB_ENDPOINT_COUNT_MAX];
    chanend_t c_ep_in[RTOS_USB_ENDPOINT_COUNT_MAX];
    c_ep_out[0] = c_ep0_Out;
    c_ep_in[0] = c_ep0_In;
    
    // TODO If noEpOut and noEpIn are greater than 1, the channels for other endpoints need to be allocated here.
    // The other endpoints will also need to be signalled, to call XUD_InitEp() now.

    printf("In _XUD_Main(): noEpOut = %lu, noEpIn = %lu\n", noEpOut, noEpIn);
    printf("In _XUD_Main(), 0x%x, 0x%x\n", epTypeTableOut[0], epTypeTableIn[0]);
    hwtimer_realloc_xc_timer();

    XUD_Main(c_ep_out, noEpOut, c_ep_in, noEpIn, 0,
        epTypeTableOut, epTypeTableIn, desiredSpeed, pwrConfig);
    hwtimer_free_xc_timer();
}