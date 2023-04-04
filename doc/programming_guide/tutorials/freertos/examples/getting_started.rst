.. _freertos-getting-started:

################################
FreeRTOS Getting Started Example
################################

This is a simple multi-tile FreeRTOS example application. It starts the FreeRTOS scheduler running on both `tile[0]` and `tile[1]`.  `tile[0]` has two tasks running.  One prints "Hello from tile 0" to the console and the other task blinks the 4 LEDs on the Explorer board.  `tile[1]` has one task running that prints "Hello from tile 0" to the console.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_sdk root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_freertos_getting_started

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    make run_example_freertos_getting_started

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_sdk root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_freertos_getting_started

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    nmake run_example_freertos_getting_started

******************
Application output
******************

If successful, you should see the following print messages in the console:

.. code-block:: console

    Hello from tile 0
    Hello from tile 1
    Hello from tile 0
    Hello from tile 1
    Hello from tile 0
    Hello from tile 1

And all four LEDs should be blinking. 

**Congratulations!**  You've just built and run your first application for xcore.  
