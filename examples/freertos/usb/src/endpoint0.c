// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

#include <string.h>

#include "rtos_usb.h"
#include "class/hid.h"



/* USB HID Device Product Defines */
#define BCD_DEVICE   0x1000
#define VENDOR_ID    0x20B1
#define PRODUCT_ID   0x1010

/* Standard HID Request Defines */

/* 7. Requests */

/* 7.1 Standard Requests  - Class Descriptor Types - High byte of wValue
 * The following defines valid types of Class descriptors */

#define HID_HID                   0x2100
#define HID_REPORT                0x2200
#define HID_PHYSICAL_DESCRIPTOR   0x2300
/* 0x24 - 0x2F: Reserved */

/* 7.2 Class-Specific Requests - bRequest values */
#define HID_GET_REPORT            0x01           /* Mandatory */
#define HID_GET_IDLE              0x02
#define HID_GET_PROTOCOL          0x03           /* Required only for boot devices */
/* Ox04 - 0x08 reserved */
#define HID_SET_REPORT            0x09
#define HID_SET_IDLE              0x0A
#define HID_SET_PROTOCOL          0x0B           /* Required only for boot devices */

/* Device Descriptor */
static unsigned char devDesc[] =
{
    0x12,                  /* 0  bLength */
    USB_DESCTYPE_DEVICE,   /* 1  bdescriptorType */
    0x00,                  /* 2  bcdUSB */
    0x02,                  /* 3  bcdUSB */
    0x00,                  /* 4  bDeviceClass */
    0x00,                  /* 5  bDeviceSubClass */
    0x00,                  /* 6  bDeviceProtocol */
    0x40,                  /* 7  bMaxPacketSize */
    (VENDOR_ID & 0xFF),    /* 8  idVendor */
    (VENDOR_ID >> 8),      /* 9  idVendor */
    (PRODUCT_ID & 0xFF),   /* 10 idProduct */
    (PRODUCT_ID >> 8),     /* 11 idProduct */
    (BCD_DEVICE & 0xFF),   /* 12 bcdDevice */
    (BCD_DEVICE >> 8),     /* 13 bcdDevice */
    0x01,                  /* 14 iManufacturer */
    0x02,                  /* 15 iProduct */
    0x00,                  /* 16 iSerialNumber */
    0x01                   /* 17 bNumConfigurations */
};


/* Configuration Descriptor */
static unsigned char cfgDesc[] = {
    0x09,                 /* 0  bLength */
    0x02,                 /* 1  bDescriptortype */
    0x22, 0x00,           /* 2  wTotalLength */
    0x01,                 /* 4  bNumInterfaces */
    0x01,                 /* 5  bConfigurationValue */
    0x03,                 /* 6  iConfiguration */
    0x80,                 /* 7  bmAttributes */
    0xC8,                 /* 8  bMaxPower */

    0x09,                 /* 0  bLength */
    0x04,                 /* 1  bDescriptorType */
    0x00,                 /* 2  bInterfacecNumber */
    0x00,                 /* 3  bAlternateSetting */
    0x01,                 /* 4: bNumEndpoints */
    0x03,                 /* 5: bInterfaceClass */
    0x00,                 /* 6: bInterfaceSubClass */
    0x02,                 /* 7: bInterfaceProtocol*/
    0x00,                 /* 8  iInterface */

    0x09,                 /* 0  bLength. Note this is currently
                                replicated in hidDescriptor[] below */
    0x21,                 /* 1  bDescriptorType (HID) */
    0x10,                 /* 2  bcdHID */
    0x11,                 /* 3  bcdHID */
    0x00,                 /* 4  bCountryCode */
    0x01,                 /* 5  bNumDescriptors */
    0x22,                 /* 6  bDescriptorType[0] (Report) */
    0x48,                 /* 7  wDescriptorLength */
    0x00,                 /* 8  wDescriptorLength */

    0x07,                 /* 0  bLength */
    0x05,                 /* 1  bDescriptorType */
    0x81,                 /* 2  bEndpointAddress */
    0x03,                 /* 3  bmAttributes */
    0x40,                 /* 4  wMaxPacketSize */
    0x00,                 /* 5  wMaxPacketSize */
    0x01                  /* 6  bInterval */
};

static unsigned char hidDescriptor[] =
{
    0x09,               /* 0  bLength */
    0x21,               /* 1  bDescriptorType (HID) */
    0x10,               /* 2  bcdHID */
    0x11,               /* 3  bcdHID */
    0x00,               /* 4  bCountryCode */
    0x01,               /* 5  bNumDescriptors */
    0x22,               /* 6  bDescriptorType[0] (Report) */
    0x48,               /* 7  wDescriptorLength */
    0x00,               /* 8  wDescriptorLength */
};

/* HID Report Descriptor */
static unsigned char hidReportDescriptor[] =
{
    0x05, 0x01,          // Usage page (desktop)
    0x09, 0x02,          // Usage (mouse)
    0xA1, 0x01,          // Collection (app)
    0x05, 0x09,          // Usage page (buttons)
    0x19, 0x01,
    0x29, 0x03,
    0x15, 0x00,          // Logical min (0)
    0x25, 0x01,          // Logical max (1)
    0x95, 0x03,          // Report count (3)
    0x75, 0x01,          // Report size (1)
    0x81, 0x02,          // Input (Data, Absolute)
    0x95, 0x01,          // Report count (1)
    0x75, 0x05,          // Report size (5)
    0x81, 0x03,          // Input (Absolute, Constant)
    0x05, 0x01,          // Usage page (desktop)
    0x09, 0x01,          // Usage (pointer)
    0xA1, 0x00,          // Collection (phys)
    0x09, 0x30,          // Usage (x)
    0x09, 0x31,          // Usage (y)
    0x15, 0x81,          // Logical min (-127)
    0x25, 0x7F,          // Logical max (127)
    0x75, 0x08,          // Report size (8)
    0x95, 0x02,          // Report count (2)
    0x81, 0x06,          // Input (Data, Rel=0x6, Abs=0x2)
    0xC0,                // End collection
    0x09, 0x38,          // Usage (Wheel)
    0x95, 0x01,          // Report count (1)
    0x81, 0x02,          // Input (Data, Relative)
    0x09, 0x3C,          // Usage (Motion Wakeup)
    0x15, 0x00,          // Logical min (0)
    0x25, 0x01,          // Logical max (1)
    0x75, 0x01,          // Report size (1)
    0x95, 0x01,          // Report count (1)
    0xB1, 0x22,          // Feature (No preferred, Variable)
    0x95, 0x07,          // Report count (7)
    0xB1, 0x01,          // Feature (Constant)
    0xC0                 // End collection
};

/* String table */
static char *stringDescriptors[]=
{
    "\x09\x04",             // Language ID string (US English)
    "XMOS",                 // iManufacturer
    "Example HID Mouse",    // iProduct
    "Config",               // iConfiguration
};

/* HID Class Requests */
XUD_Result_t HidInterfaceClassRequests(rtos_usb_t *ctx,
                                       XUD_ep c_ep0_out,
                                       XUD_ep c_ep0_in,
                                       USB_SetupPacket_t sp)
{
    extern uint8_t g_reportBuffer[4];
    uint8_t buffer[64];

    switch (sp.bRequest)
    {
    case HID_GET_REPORT:
        memcpy(buffer, g_reportBuffer, 4);

        rtos_printf("HID_GET_REPORT: %d bytes\n", sp.wLength);
        return XUD_DoGetRequest(c_ep0_out, c_ep0_in, buffer, 4, sp.wLength);
        break;

    case HID_GET_IDLE:
        /* Return the current Idle rate - optional for a HID mouse */

        /* Do nothing - i.e. STALL */
        rtos_printf("HID_GET_IDLE\n");
        break;

    case HID_GET_PROTOCOL:
        /* Required only devices supporting boot protocol devices,
         * which this example does not */

        /* Do nothing - i.e. STALL */
        rtos_printf("HID_GET_PROTOCOL\n");
        break;

    case HID_SET_REPORT:
        /* The host sends an Output or Feature report to a HID
         * using a cntrol transfer - optional */

        /* Do nothing - i.e. STALL */
        rtos_printf("HID_SET_REPORT\n");
        break;

    case HID_SET_IDLE:
        /* Set the current Idle rate - this is optional for a HID mouse
         * (Bandwidth can be saved by limiting the frequency that an
         * interrupt IN EP when the data hasn't changed since the last
         * report */

        /* Do nothing - i.e. STALL */
        rtos_printf("HID_SET_IDLE - requested %d\n", sp.wLength);

        if (
        rtos_usb_endpoint_transfer_start(ctx,
                                         0x80,
                                         buffer,
                                         0) == XUD_RES_OKAY &&

        rtos_usb_endpoint_transfer_complete(ctx,
                                            0x80,
                                            NULL,
                                            RTOS_OSAL_WAIT_FOREVER) == XUD_RES_OKAY) {
            return XUD_RES_OKAY;
        }

//            return XUD_DoGetRequest(c_ep0_out, c_ep0_in, buffer, 0, sp.wLength);

//            return XUD_SetBuffer(c_ep0_in, buffer, 0);
//            XUD_SetStall(c_ep0_in);

        break;

    case HID_SET_PROTOCOL:
        /* Required only devices supporting boot protocol devices,
         * which this example does not */

        /* Do nothing - i.e. STALL */
        rtos_printf("HID_SET_PROTOCOL\n");
        break;
    }

    return XUD_RES_ERR;
}



void endpoint_0(rtos_usb_t *ctx)
{
    USB_SetupPacket_t sp;

    unsigned bmRequestType;
    XUD_BusSpeed_t usbBusSpeed;

    rtos_usb_endpoint_ready(ctx, 0x00, RTOS_OSAL_WAIT_FOREVER);
    rtos_usb_endpoint_ready(ctx, 0x80, RTOS_OSAL_WAIT_FOREVER);
    XUD_ep ep0_out = ctx->ep[0][0];
    XUD_ep ep0_in = ctx->ep[0][1];

    while (1)
    {
        uint8_t setup_packet[10];
        size_t len;
        XUD_Result_t result;
        /* Returns XUD_RES_OKAY on success */

        result = rtos_usb_endpoint_transfer_start(ctx,
                                                  0x00,
                                                  setup_packet,
                                                  8);
        if (result == XUD_RES_OKAY) {
            result = rtos_usb_endpoint_transfer_complete(ctx,
                                                         0x00,
                                                         &len,
                                                         RTOS_OSAL_WAIT_FOREVER);
        }
        if (result == XUD_RES_OKAY) {
            xassert(len == 8);

            USB_ParseSetupPacket(setup_packet,
                                 &sp);
        }

        if(result == XUD_RES_OKAY)
        {
            /* Set result to ERR, we expect it to get set to OKAY if a request is handled */
            result = XUD_RES_ERR;

            /* Stick bmRequest type back together for an easier parse... */
            bmRequestType = (sp.bmRequestType.Direction<<7) |
                            (sp.bmRequestType.Type<<5) |
                            (sp.bmRequestType.Recipient);

            if ((bmRequestType == USB_BMREQ_H2D_STANDARD_DEV) &&
                (sp.bRequest == USB_SET_ADDRESS))
            {
              // Host has set device address, value contained in sp.wValue
            }

            switch(bmRequestType)
            {
                /* Direction: Device-to-host
                 * Type: Standard
                 * Recipient: Interface
                 */
                case USB_BMREQ_D2H_STANDARD_INT:

                    if(sp.bRequest == USB_GET_DESCRIPTOR)
                    {
                        /* HID Interface is Interface 0 */
                        if(sp.wIndex == 0)
                        {
                            /* Look at Descriptor Type (high-byte of wValue) */
                            unsigned short descriptorType = sp.wValue & 0xff00;

                            switch(descriptorType)
                            {
                                case HID_HID:
                                    result = XUD_DoGetRequest(ep0_out, ep0_in, hidDescriptor, sizeof(hidDescriptor), sp.wLength);
                                    break;

                                case HID_REPORT:
                                    result = XUD_DoGetRequest(ep0_out, ep0_in, hidReportDescriptor, sizeof(hidReportDescriptor), sp.wLength);
                                    break;
                            }
                        }
                    }
                    break;

                /* Direction: Device-to-host and Host-to-device
                 * Type: Class
                 * Recipient: Interface
                 */
                case USB_BMREQ_H2D_CLASS_INT:
                case USB_BMREQ_D2H_CLASS_INT:

                    /* Inspect for HID interface num */
                    if(sp.wIndex == 0)
                    {
                        /* Returns  XUD_RES_OKAY if handled,
                         *          XUD_RES_ERR if not handled,
                         *          XUD_RES_RST for bus reset */
                        result = HidInterfaceClassRequests(ctx, ep0_out, ep0_in, sp);
                    }
                    break;
            }
        }

        /* If we haven't handled the request about then do standard enumeration requests */
        if(result == XUD_RES_ERR )
        {
            /* Returns  XUD_RES_OKAY if handled okay,
             *          XUD_RES_ERR if request was not handled (STALLed),
             *          XUD_RES_RST for USB Reset */
            result = USB_StandardRequests(ep0_out,
                                          ep0_in,
                                          devDesc,
                                          sizeof(devDesc),
                                          cfgDesc,
                                          sizeof(cfgDesc),
                                          devDesc,
                                          sizeof(devDesc),
                                          cfgDesc,
                                          sizeof(cfgDesc),
                                          stringDescriptors,
                                          sizeof(stringDescriptors) / sizeof(stringDescriptors[0]),
                                          &sp,
                                          usbBusSpeed);
        }

        /* USB bus reset detected, reset EP and get new bus speed */
        if(result == XUD_RES_RST)
        {
            usbBusSpeed = XUD_ResetEndpoint(ep0_out, &ep0_in);
        }
    }
}
