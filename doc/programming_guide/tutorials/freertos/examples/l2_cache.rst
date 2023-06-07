################
L2 Cache Example
################

The L2 cache example demonstrates how to use the software defined L2 cache.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_freertos_l2_cache

=======================
Setting up the hardware
=======================

Before running the firmware, the swmem must be flashed.

.. code-block:: console

    make flash_example_freertos_l2_cache_swmem

====================
Running the firmware
====================

Running with hardware.

.. code-block:: console

    make run_example_freertos_l2_cache

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_freertos_l2_cache


=======================
Setting up the hardware
=======================

Before running the firmware, the swmem must be flashed.

.. code-block:: console

    nmake flash_example_freertos_l2_cache_swmem

====================
Running the firmware
====================

Running with hardware.

.. code-block:: console

    nmake run_example_freertos_l2_cache
