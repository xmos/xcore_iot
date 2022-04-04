============================
FFD Reference Design
============================

This is the XMOS FFD reference design.

******************
Supported Hardware
******************

This example is supported on the XK_VOICE_L71 board.

*********************
Building the Firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make application_ffd

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake application_ffd


********************
Running the Firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_application_ffd

.. tab:: Windows

    .. code-block:: console

        $ nmake run_application_ffd


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_application_ffd

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_application_ffd
