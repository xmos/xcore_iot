################
GPIO RTOS Driver
################

This driver can be used to operate GPIO ports on XCore in an RTOS application.

******************
Initialization API
******************
The following structures and functions are used to initialize and start a GPIO driver instance.

.. doxygengroup:: rtos_gpio_driver
   :content-only:

********
Core API
********

The following functions are the core GPIO driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_gpio_driver_core
   :content-only:

**********************
RPC Initialization API
**********************

The following functions may be used to share a GPIO driver instance with other XCore tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_gpio_driver_rpc
   :content-only:
