// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DEMO_MAIN_H_
#define DEMO_MAIN_H_

/* Must be called before usb_manager_start() */
void create_tinyusb_disks(void *args);

void create_tinyusb_demo(void *args, unsigned priority);

#endif /* DEMO_MAIN_H_ */
