##############
DFU
##############

This example application demonstrates a method to add DFU to a FreeRTOS application on XCORE.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_dfu

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_dfu


********************
Running the firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_freertos_dfu

.. tab:: Windows

    .. code-block:: console

        nmake run_example_freertos_dfu


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_example_freertos_dfu

.. tab:: Windows

    .. code-block:: console

        nmake debug_example_freertos_dfu
