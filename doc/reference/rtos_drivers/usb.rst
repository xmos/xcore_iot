#####################
USB RTOS Driver
#####################

This driver can be used to instantiate and control a USB device interface on XCore in an RTOS application.

Unlike most other XCore I/O interface RTOS drivers, only a single USB driver instance may be started. It also does not require an initialization step prior to starting the driver. This is due to an implementation detail in lib_xud, which is what the RTOS USB driver uses at its core.

**********
Driver API
**********
The following structures and functions are used to start and use a USB driver instance.

.. doxygengroup:: rtos_usb_driver
   :content-only:
   :inner:
