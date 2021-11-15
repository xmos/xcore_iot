#####################
Intertile RTOS Driver
#####################

This driver allows for communication between AMP RTOS instances running on different XCore tiles.

******************
Initialization API
******************
The following structures and functions are used to initialize and start an intertile driver instance.

.. doxygengroup:: rtos_intertile_driver
   :content-only:

********
Core API
********

The following functions are the core intertile driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_intertile_driver_core
   :content-only:
