########################
FreeRTOS Getting Started
########################

This is the simplest buildable multitile FreeRTOS project for XCore. We encourage you to use this as a template for new projects. To start your own project copy the contents of this folder to your computer and begin developing.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_getting_started

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_getting_started

*********************
Running the firmware
*********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_getting_started

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_getting_started

********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_example_freertos_getting_started

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_example_freertos_getting_started
