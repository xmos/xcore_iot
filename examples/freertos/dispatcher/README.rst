##################
Dispatcher Example
##################

The dispatcher example is demonstrates how to use the dispatcher to parallelize a matrix multiplication operation. Matrix multiplication is a data parallel operation. This means the input matrices can be partitioned and the multiplication operation run on the individual partitions in parallel. A dispatcher is well suited for data parallel problems.

Note

The function used in this example to multiply two matrices is for illustrative use only. It is not the most efficient way to perform a matrix multiplication. XMOS has optimized libraries specifically for this purpose.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_dispatcher

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_dispatcher

********************
Running the firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_dispatcher

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_dispatcher


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_example_freertos_dispatcher

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_example_freertos_dispatcher
