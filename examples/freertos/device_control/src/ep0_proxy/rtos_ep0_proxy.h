#ifndef RTOS_EP0_H
#define RTOS_EP0_H

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include "rtos_osal.h"
#include "xud_device.h"
#include "rtos_usb.h"

typedef enum
{
    EP0_TRANSFER_COMPLETE = 8,
    EP0_PROXY_CMD
}e_ep0_proxy_event_t;

typedef struct
{
  uint8_t rhport;
  uint8_t event_id;

  union
  {
    // XFER_COMPLETE
    struct {
      uint8_t ep_num;
      uint8_t dir;
      XUD_Result_t result;
      uint8_t is_setup;
      uint32_t len;
    }xfer_complete;

    // FUNC_CALL
    struct {
      uint8_t cmd;
      uint8_t proxy_dir;
    }ep0_command;
  };
} ep0_proxy_event_t;


void ep0_proxy_init(
    chanend_t chan_ep0_out,
    chanend_t chan_ep0_in,
    chanend_t chan_ep0_out_proxy,
    chanend_t chan_ep_hid_proxy,
    chanend_t c_ep0_proxy_xfer_complete);

void ep0_proxy_task(void *app_data);

void ep0_proxy_start(unsigned priority);

#endif