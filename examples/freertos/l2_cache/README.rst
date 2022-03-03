################
L2 Cache Example
################

The L2 cache example demonstrates how to use the software defined L2 cache.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_l2_cache

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_l2_cache


***********************
Setting up the hardware
***********************

Before running the firmware, the swmem must be flashed.

.. tab:: Linux and Mac

    .. code-block:: console

        $ make flash_example_freertos_l2_cache_swmem

.. tab:: Windows

    .. code-block:: console

        $ nmake flash_example_freertos_l2_cache_swmem

********************
Running the firmware
********************

Running with hardware.

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_l2_cache

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_l2_cache
