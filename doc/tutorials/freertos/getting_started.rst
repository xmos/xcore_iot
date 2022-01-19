.. include:: ../../substitutions.rst

#############################
Getting Started with FreeRTOS
#############################

This document is intended to help you start your first FreeRTOS application on XCore.  We assume you have read :doc:`FreeRTOS Application Programming <application_programming>` and that you are familiar with FreeRTOS. 

***********************
RTOS Application Design
***********************

A fully functional example application that can be found in the SDK under the path `examples/freertos/explorer_board <https://github.com/xmos/xcore_sdk/tree/develop/examples/freertos/explorer_board>`_. This application does not provide a complete example of how to use all of the RTOS drivers. It is, however, a reference for how to use most of the drivers, it does utilizes many of the software services, and it also serves as an example for how to structure an SMP RTOS application for XCore.

This example application runs two instances of SMP FreeRTOS, one on each of the processor's two tiles. Because each tile has its own memory which is not shared between them, this can be viewed as a single asymmetric multiprocessing (AMP) system that comprises two SMP systems. A FreeRTOS thread that is created on one tile will never be scheduled to run on the other tile. Similarly, an RTOS object that is created on a tile, such as a queue, can only be accessed by threads and ISRs that run on that tile and never by code running on the other tile.

That said, the example application is programmed and built as a single coherent application, which will be familiar to programmers who have previously programmed for the XCore in XC. Data that must be shared between threads running on different tiles is sent via a channel using the RTOS intertile driver, which under the hood uses a streaming channel between the tiles.

Most of the I/O interface drivers in fact provide a mechanism to share driver instances between tiles that utilizes this intertile driver. For those familiar with XC, this can be viewed as a C alternative to XC interfaces.

For example, a SPI interface might be available on tile 0. Normally, initialization code that runs on tile 0 sets this interface up and then starts the driver. Without any further initialization, code that runs on tile 1 will be unable to access this interface directly, due both to not having direct access to tile 0's memory, as well as not having direct access to tile 0's ports. The drivers, however, provide some additional initialization functions that can be used by the application to share the instance on tile 0 with tile 1. After this initialization is done, code running on tile 1 may use the instance with the same driver API as tile 0, almost as if it was actually running on tile 0.

The example application referenced above, as well as the RTOS driver documentation, should be consulted to see exactly how to initialize and share driver instances.

The SDK provides the ON_TILE(t) preprocessor macro. This macro may be used by applications to ensure certain code is included only on a specific tile at compile time. In the example application, there is a single task that is created on both tiles that starts the drivers and creates the remaining application tasks. While this function is written as a single function, various parts are inside #if ON_TILE() blocks. For example, consider the following code snippet found inside the task vApplicationDaemonTaskStartup():

.. code-block:: C

  #if ON_TILE(I2C_TILE)
  {
      int dac_init(rtos_i2c_master_t *i2c_ctx);
      if (dac_init(i2c_master_ctx) == 0) {
          rtos_printf("DAC initialization succeeded\n");
          dac_configured = 1;
      } else {
          rtos_printf("DAC initialization failed\n");
          dac_configured = 0;
      }
      chan_out_byte(other_tile_c, dac_configured);
  }
  #else
  {
      dac_configured = chan_in_byte(other_tile_c);
  }
  #endif

When this function is compiled for tile I2C_TILE, only the first block is included. When it is compiled for the other tile, only the second block is included. When the application is run, tile I2C_TILE performs the initialization of the DAC, while the other tile waits for the DAC initialization to complete.

I2C_TILE is defined at the top of the file. Because the |I2C| driver instance is shared between the two tiles, it may in fact be set to either zero or one, providing a demonstration of the way that drivers instances may be shared between tiles.

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

A minimal example application that can be used as a template to begin a new project can be found in the SDK under the path ``examples/freertos/getting_started``.

**************************
Building RTOS Applications
**************************

RTOS applications using the SDK are built using `CMake`. The SDK provides many drivers and services, all of which can be included by the application's `CMakeLists.txt` file. The application's CMakeLists can specify precisely which drivers and software services within the SDK should be included through the use of various CMake options.  

The example applications also provide a GNU Make Makefile that actually runs CMake and then runs make with the generated CMake makefiles. This is done by simply running:

.. code-block:: console

  $ make -j

in the example application directories, all the steps necessary to build the entire application are taken, and a single binary that includes both tiles will be found under the bin directory. If the XCore board is connected to the computer via an xTag, running:

.. code-block:: console

  $ make run

will run it on the board with xscope enabled so that all debug output from the application will be routed to the terminal.

All applications have a README file with additional instructions on how to setup, build and run the application.  See the :doc:`FreeRTOS Examples <examples/index>` page for a list of all example applications.

Including the RTOS Platform
===========================

The simplest way to specify commonly used RTOS sources is to include the `xmos_rtos_platform.cmake` in your applications `CMakeLists.txt` file.

.. code-block:: CMake

  include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

Then, later in your `CMakeLists.txt` file, set the target sources and includes using the `XMOS_RTOS_PLATFORM_SOURCES` variable and your own `APP_SOURCES` variable.

.. code-block:: CMake

  target_sources(my_target_name PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
  target_include_directories(my_target_name PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

See the :ref:`Build System <sdk-build_system-label>` page for more information on the CMake-based build system.  The `CMakeLists.txt` files in the FreeRTOS example applications are another great places to see for how this is implemented in a realistic project.

Build Variables
===============

Including the `xmos_rtos_platform.cmake` is all that most applications need to implement.  However, your application may use modules that are not enabled by default.

To include those modules, additional build variables must be set in the `CMakeLists.txt` file before including `xmos_rtos_platform.cmake`. See the example below:

.. code-block:: CMake

  set(USE_TINYUSB TRUE)

  include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

In addition, your application may not use modules that are included by default. Modules can be excluded, potentially speeding up builds. See the example below:

.. code-block:: CMake

  set(USE_TINYUSB TRUE)
  set(USE_RTOS_I2S_DRIVER FALSE)
  set(USE_RTOS_SWMEM_DRIVER FALSE)

  include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

See :ref:`CMake Variables <sdk-cmake-variables-label>` for the full set of supported build variables, including the default setting for each.
