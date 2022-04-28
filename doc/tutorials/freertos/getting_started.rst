.. include:: ../../substitutions.rst

#############################
Getting Started with FreeRTOS
#############################

This document is intended to help you start your first FreeRTOS application on XCore.  We assume you have read :doc:`FreeRTOS Application Programming <application_programming>` and that you are familiar with FreeRTOS. 

***********************
RTOS Application Design
***********************

A fully functional example application that can be found in the SDK under the path `examples/freertos/explorer_board <https://github.com/xmos/xcore_sdk/tree/develop/examples/freertos/explorer_board>`_. This application does not provide a complete example of how to use all of the RTOS drivers. It is, however, a reference for how to use most of the drivers, it does utilizes many of the software services, and it also serves as an example for how to structure an SMP RTOS application for XCore.  Additional code to initialize the SoC platform for this example is provided by a board support library `modules/rtos/board_support/XCORE-AI-EXPLORER_2V0/platform <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/bsp_config/XCORE-AI-EXPLORER_2V0/platform>`_

This example application runs two instances of SMP FreeRTOS, one on each of the processor's two tiles. Because each tile has its own memory which is not shared between them, this can be viewed as a single asymmetric multiprocessing (AMP) system that comprises two SMP systems. A FreeRTOS thread that is created on one tile will never be scheduled to run on the other tile. Similarly, an RTOS object that is created on a tile, such as a queue, can only be accessed by threads and ISRs that run on that tile and never by code running on the other tile.

That said, the example application is programmed and built as a single coherent application, which will be familiar to programmers who have previously programmed for the XCore in XC. Data that must be shared between threads running on different tiles is sent via a channel using the RTOS intertile driver, which under the hood uses a streaming channel between the tiles.

Most of the I/O interface drivers in fact provide a mechanism to share driver instances between tiles that utilizes this intertile driver. For those familiar with XC, this can be viewed as a C alternative to XC interfaces.

For example, a SPI interface might be available on tile 0. Normally, initialization code that runs on tile 0 sets this interface up and then starts the driver. Without any further initialization, code that runs on tile 1 will be unable to access this interface directly, due both to not having direct access to tile 0's memory, as well as not having direct access to tile 0's ports. The drivers, however, provide some additional initialization functions that can be used by the application to share the instance on tile 0 with tile 1. After this initialization is done, code running on tile 1 may use the instance with the same driver API as tile 0, almost as if it was actually running on tile 0.

The example application referenced above, as well as the RTOS driver documentation, should be consulted to see exactly how to initialize and share driver instances.

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

The application may be experimented with by modifying the \*RPC_ENABLED macros in `app_conf.h`, as well as the \*_TILE macros at the top of `driver_instances.c`. RPC here stands for Remote Procedure Call, and is what allows for driver instances to be shared. Provided RPC is enabled for a particular driver, it may be used by either tile and the corresponding \*_TILE macros for it may be set to either tile. However, if RPC is disabled then note that when the corresponding \*_TILE macro is not set to the tile that owns the instance, the application will fail.

Consult the RTOS driver documentation for the details on what exactly each of the RTOS API functions called by this application does.

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
