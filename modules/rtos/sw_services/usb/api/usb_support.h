// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

#ifndef USB_SUPPORT_H_
#define USB_SUPPORT_H_

/**
 * Initializes and starts the TinyUSB stack
 *
 * \param priority The priority to use for the USB task.
 */
void usb_manager_start(unsigned priority);

#endif /* USB_SUPPORT_H_ */
