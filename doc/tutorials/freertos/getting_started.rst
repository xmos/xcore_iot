.. include:: ../../substitutions.rst

#############################
Getting Started with FreeRTOS
#############################

This document is intended to help you start your first FreeRTOS application on XCore.  We assume you have read :doc:`FreeRTOS Application Programming <application_programming>` and that you are familiar with FreeRTOS.

***********************
RTOS Application Design
***********************

A fully functional example application that can be found in the SDK under the path `examples/freertos/explorer_board <https://github.com/xmos/xcore_sdk/tree/develop/examples/freertos/explorer_board>`_. This application is a reference for how to use an RTOS drivers or software service, and serves as an example for how to structure an SMP RTOS application for XCore.  Additional code to initialize the SoC platform for this example is provided by a board support configuration library `modules/rtos/board_support/XCORE-AI-EXPLORER_2V0/platform <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/bsp_config/XCORE-AI-EXPLORER_2V0/platform>`_

This example application runs two instances of SMP FreeRTOS, one on each of the processor's two tiles. Because each tile has its own memory which is not shared between them, this can be viewed as a single asymmetric multiprocessing (AMP) system that comprises two SMP systems. A FreeRTOS thread that is created on one tile will never be scheduled to run on the other tile. Similarly, an RTOS object that is created on a tile, such as a queue, can only be accessed by threads and ISRs that run on that tile and never by code running on the other tile.

That said, the example application is programmed and built as a single coherent application, which will be familiar to programmers who have previously programmed for the XCore in the XC programming language. Data that must be shared between threads running on different tiles is sent via a channel using the RTOS intertile driver, which under the hood uses a streaming channel between the tiles.

Most of the I/O interface drivers in fact provide a mechanism to share driver instances between tiles that utilizes this intertile driver. For those familiar with XC programming, this can be viewed as a C alternative to XC interfaces.

For example, a SPI interface might be available on tile 0. Normally, initialization code that runs on tile 0 sets this interface up and then starts the driver. Without any further initialization, code that runs on tile 1 will be unable to access this interface directly, due both to not having direct access to tile 0's memory, as well as not having direct access to tile 0's ports. The drivers, however, provide some additional initialization functions that can be used by the application to share the instance on tile 0 with tile 1. After this initialization is done, code running on tile 1 may use the instance with the same driver API as tile 0, almost as if it was actually running on tile 0.

The example application referenced above, as well as the RTOS driver documentation, should be consulted to see exactly how to initialize and share driver instances.  Additionally, not all IO is capable of being shared between tiles directly through the driver API due to timing constraints.

The SDK provides the ON_TILE(t) preprocessor macro. This macro may be used by applications to ensure certain code is included only on a specific tile at compile time. In the example application, there is a single task that is created on both tiles that starts the drivers and creates the remaining application tasks. While this function is written as a single function, various parts are inside #if ON_TILE() blocks. For example, consider the following code snippet found inside the i2c_init() `function <https://github.com/xmos/xcore_sdk/blob/develop/modules/rtos/bsp_config/XCORE-AI-EXPLORER_2V0/platform/platform_init.c>`_:

.. code-block:: C

    #if ON_TILE(I2C_TILE_NO)
        rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};
        rtos_i2c_master_init(
                i2c_master_ctx,
                PORT_I2C_SCL, 0, 0,
                PORT_I2C_SDA, 0, 0,
                0,
                100);

        rtos_i2c_master_rpc_host_init(
                i2c_master_ctx,
                &i2c_rpc_config,
                client_intertile_ctx,
                1);
    #else
        rtos_i2c_master_rpc_client_init(
                i2c_master_ctx,
                &i2c_rpc_config,
                intertile_ctx);
    #endif

When this function is compiled for tile I2C_TILE_NO, only the first block is included. When it is compiled for the other tile, only the second block is included. When the application is run, tile I2C_TILE_NO performs the initialization of the the |I2C| master driver host, while the other tile initializes the |I2C| master driver client. Because the |I2C| driver instance is shared between the two tiles, it may in fact be set to either zero or one, providing a demonstration of the way that drivers instances may be shared between tiles.

The SDK provides a single XC file that provides the `main()` function. This provided `main()` function calls `main_tile0()` through `main_tile3()`, depending on the number of tiles that the application requires and the number of tiles provided by the target XCore processor. The application must provide each of these tile entry point functions. Each one is provided with up to three channel ends that are connected to each of the other tiles.

The example application provides both `main_tile0()` and `main_tile1()`. Each one calls a common initialization function that initializes all the drivers for the interfaces specific to its tile. These functions also call the initialization functions to share these driver instances between the tiles. These initialization functions are found in the `platform/platform_init.c` source file.

Each tile then creates the `startup_task()` task and starts the FreeRTOS scheduler. The `startup_task()` completes the driver instance sharing and then starts all of the driver instances. The driver startup functions are found in the `platform/platform_start.c` source file.

Consult the RTOS driver documentation for the details on what exactly each of the RTOS API functions called by this application does.

****************************
Board Support Configurations
****************************

XCore leverages its architecture to provide a flexible chip where many typically silicon based peripherals are found in software. This allows a chip to be reconfigured in a way that provides the specific IO required for a given application, thus resulting in a low cost yet incredibly silicon efficient solution. Board support configurations (bsp_configs) are the description for the hardware IO that exists in a given board. The bsp_configs provide the application programmer with an API to initialize and start the hardware configuration, as well as the supported RTOS driver contexts. The programming model in this FreeRTOS architecture is:
    * .xn files provide the mapping of ports, pins, and links
    * bsp_configs setup and start hardware IO and provide the application with RTOS driver contexts
    * applications use the bsp_config init/start code as well as RTOS driver contexts, similar to conventional microcontroller programming models.


To support any generic bsp_config, applications should call `platform_init()` before starting the scheduler, and then `platform_start()` after the scheduler is running and before any RTOS drivers are used.

The bsp_configs provided with the XCore SDK in `modules/rtos/bsp_config <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/bsp_config>`_ are an excellent starting point. They provide the most common peripheral drivers that are supported by the boards that support the XCore SDK. For advanced users, it is recommended that you copy one of these bsp_config into your application project and customize as needed. See `application/ffd/bsp_config <https://github.com/xmos/xcore_sdk/tree/develop/applications/ffd/bsp_config>`_ for example of how the XCORE_AI_EXPLORER bsp_config has been customized for an application.  

*******************************
Developing and Debugging Memory
*******************************

The XTC Tools provide compile time information to aid developers in creating and testing of their application.

==============
Resource Usage
==============

One of these features if the `-report` option, which will `Display a summary of resource usage`. One of the outputs of this report is memory usage, split into the stack, code, and data requirements of the program.  Unlike most XC applications, FreeRTOS makes heavy use of dynamic memory allocation. The FreeRTOS heap will appear as `Data` in the XTC Tools report. The heap size is determined by the compile time definition `configTOTAL_HEAP_SIZE`, which can be found in an application's FreeRTOSConfig.h.

For AMP SMP FreeRTOS builds, which are created using the cmake macro `merge_binaries()`, there are actually multiple application builds, one per tile, which are then combined. While building a given AMP application, the console output will contain both of the individual tile build reports.

As an example, consider building the `example_freertos_explorer_board` target.

.. code-block:: console

    Constraint check for tile[0]:
      Memory available:       524288,   used:      318252 .  OKAY
        (Stack: 5260, Code: 42314, Data: 270678)
    Constraints checks PASSED WITH CAVEATS.
    Constraint check for tile[1]:
      Memory available:       524288,   used:       4060 .  OKAY
        (Stack: 356, Code: 3146, Data: 558)
    Constraints checks PASSED.

    Constraint check for tile[0]:
      Memory available:       524288,   used:       4836 .  OKAY
        (Stack: 356, Code: 3802, Data: 678)
    Constraints checks PASSED.
    Constraint check for tile[1]:
      Memory available:       524288,   used:      319476 .  OKAY
        (Stack: 14740, Code: 30730, Data: 274006)
    Constraints checks PASSED WITH CAVEATS.


In this example, the cmake contains the command:

.. code-block:: cmake

    merge_binaries(example_freertos_explorer_board tile0_example_freertos_explorer_board tile1_example_freertos_explorer_board 1)


Which means the final application usage would be interpreted as:

.. code-block:: console

    Constraint check for tile[0]:
      Memory available:       524288,   used:      318252 .  OKAY
        (Stack: 5260, Code: 42314, Data: 270678)
    Constraints checks PASSED WITH CAVEATS.
    Constraint check for tile[1]:
      Memory available:       524288,   used:      319476 .  OKAY
        (Stack: 14740, Code: 30730, Data: 274006)
    Constraints checks PASSED WITH CAVEATS.

Because the tile 1 portion of the tile1 target build replaces the tile 1 portion in the tile0 target build.

The XTC Tools also provide a method to examine the resource usage of a binary post build.  This method will only work if used on the intermediate binaries.

.. code-block:: console

    $ xobjdump --resources tile0_example_freertos_explorer_board.xe
    $ xobjdump --resources tile1_example_freertos_explorer_board.xe


Note: Because the resulting example_freertos_explorer_board.xe binary was created by merging into tile0_example_freertos_explorer_board.xe, the results of `xobjdump --resources example_freertos_explorer_board.xe` will be the exact same as `xobjdump --resources tile0_example_freertos_explorer_board.xe` and not account for the actual tile 1 requirements.

================
RTOS Stack Usage
================

Since tasks run within FreeRTOS, the RTOS stack requirement must be known at compile time.  In FreeRTOS applications on most other microcontrollers, the general practice is to create a task with a large amount of stack, use the FreeRTOS stack debug functions to determine the worst case runtime usage of stack, and then adjust the stack memory value accordingly.  The problem with this method is that the stack of any given thread varies greatly based on the functions that are called within, and thus a code or compiler optimization change result in the optimal task stack usage to have to be redetermined.  This issue results in many FreeRTOS applications being written in such a way that wastes memory, by providing task with way more stack than they should need.  Additionally, stack overflow bugs can remain hidden for a long time and even when bugs do manifest, the source can be difficult to pinpoint.

The XTC Tools address this issue by creating a symbol that represents the maximum stack requirement of any function at compile time.  By using the `RTOS_THREAD_STACK_SIZE()` macro, for the stack words argument for creating a FreeRTOS task, it is guaranteed that the optimal stack requirement is used, provided that the function does not call function pointers nor can infinitely recurse.

.. code-block:: C

    xTaskCreate((TaskFunction_t) example_task,
                "example_task",
                RTOS_THREAD_STACK_SIZE(example_task),
                NULL,
                EXAMPLE_TASK_PRIORITY,
                NULL);

If function pointers are used within a thread, then the application programmer must annotate the code with the appropriate function pointer group attribute.  For recursive functions, the only option is to specify the stack manually.  See `Appendix A - Guiding Stack Size Calculation <https://www.xmos.ai/documentation/XM-014363-PC-5/html/prog-guide/quick-start/c-programming-guide/index.html>`_ in the XTC Tools documentation for more information.

*******************
Additional Examples
*******************

Additional code examples are provided on the :doc:`FreeRTOS Examples <examples/index>` page.

********************
New Project Template
********************

A minimal example application that can be used as a template to begin a new project can be found in the SDK under the path `examples/freertos/getting_started <https://github.com/xmos/xcore_sdk/tree/develop/examples/freertos/getting_started>`_.

**************************
Building RTOS Applications
**************************

RTOS applications using the SDK are built using `CMake`. The SDK provides many libraries, drivers and software services, all of which can be included by the application's ``CMakeLists.txt`` file. The application's CMakeLists can specify precisely which drivers and software services within the SDK should be included through the use of various CMake target aliases.

See :ref:`Build System <sdk-build_system-label>` for more information on the SDK's build system.

See :ref:`Target Aliases <sdk-cmake-target-aliases>` for more information on the SDK's build system target aliases.

All SDK example applications have a README file with additional instructions on how to setup, build and run the application.  In addition, they contain  ``CMakeLists.txt`` files that can be used as starting point for a new application.  See the :doc:`FreeRTOS Examples <examples/index>` page for a list of all example applications.
