##############
Device Control
##############

This example application demonstrates how to use device control over USB and I2C.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_device_control

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_device_control

********************
Running the firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_device_control

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_device_control

********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_example_freertos_device_control

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_example_freertos_device_control

*********************
Building the host app
*********************

With the firmware running in its own terminal, in a new window,
run the following commands in the xcore_sdk root folder to build the host app:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build_host
        $ cd build_host
        $ make example_freertos_device_control_host

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build_host
        $ cd build_host
        $ nmake example_freertos_device_control_host

********************
Running the host app
********************

From the xcore_sdk/build_host/examples/freertos/device_control/host/
folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ ./example_freertos_device_control_host --help

.. tab:: Windows

    .. code-block:: console

        $ example_freertos_device_control_host.exe --help
