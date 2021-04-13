// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef USB_SUPPORT_H_
#define USB_SUPPORT_H_

/**
 * Starts the TinyUSB stack. This may be called either
 * before or after starting the RTOS scheduler.
 *
 * \param priority The priority to use for the USB task.
 */
void usb_manager_start(unsigned priority);

/**
 * Initializes the TinyUSB stack. This should be called
 * prior to starting the RTOS scheduler.
 */
void usb_manager_init(void);

#endif /* USB_SUPPORT_H_ */
