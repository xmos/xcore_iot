##############
Device Control
##############

This example application demonstrates how to use device control over USB and I2C.

*********************
Building the firmware
*********************

Run the following commands to build the device_control firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make example_freertos_device_control

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake example_freertos_device_control

*********************
Running the firmware
*********************

From the xcore_sdk build folder run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ make run_example_freertos_device_control

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_device_control

*********************
Debugging the firmware with xgdb
*********************

From the xcore_sdk build folder run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ make debug_example_freertos_device_control

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_example_freertos_device_control
