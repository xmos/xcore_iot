######################
Example CMakeLists.txt 
######################

.. note::

   CMake is powerful tool that provides the developer a great deal of flexibility in how their projects are built.  As a result, CMakeLists.txt files in the example applications may vary from the examples below.

**********
Bare-Metal
**********

Below is an example ``CMakeLists.txt`` that shows both required and conventional commands for a basic bare-metal project.

.. code-block:: cmake

    ## Set the minimum required version of cmake for your project
    ##   This is not strictly required, however, it is a good practice
    cmake_minimum_required(VERSION 3.20)

    ## Disable in-source builds
    ##   This is not strictly required, however, it is a good practice
    if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
        message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
    endif()

    ## Specify target board
    ##   Projects can target multiple boards.  If this is the case for oyur project, 
    ##   you can specify the board during the CMake configure step:
    ##   $ cmake ../ -DBOARD=XCORE-AI-EXPLORER
    set(BOARD XCORE-AI-EXPLORER)

    ## Optionally specify configuration options
    ##   See the XCore SDK documentation for the set of supported options
    set(MULTITILE_BUILD TRUE)
    set(USE_XUD_HIL TRUE)

    ## Include XMOS platform configuration
    ##   NOTE: This must be done *after* setting the board and configuration options
    include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

    ## Set your project definition
    ##   NOTE: This must be done *after* including the XMOS platform
    project(my_project)

    ## Enable the languages for your project
    enable_language(CXX C ASM)

    ## Set compiler flags as needed
    ##   Your flags may vary but below are some typical flags
    set(APP_COMPILER_FLAGS
        "-Os"
        "-report"
        "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
   )

   ## Specify your sources
   ##   NOTE: Be sure to add XMOS_PLATFORM_SOURCES to the list
   set(APP_SOURCES
      "main.c"
      ${XMOS_PLATFORM_SOURCES}
   )

   ## Specify your includes
   ##   NOTE: Be sure to add XMOS_PLATFORM_INCLUDES to the list
   set(APP_INCLUDES
      ${XMOS_PLATFORM_INCLUDES}
   )

   ## Optionally specify compile definitions as needed
   add_compile_definitions(
      DEBUG_PRINT_ENABLE=1
   )

   set(TILE_LIST 0 1)
   create_multitile_target(TILE_LIST)

********
FreeRTOS
********

Below is an example ``CMakeLists.txt`` that shows both required and conventional commands for a basic FreeRTOS project.  For a FreeRTOS project, only a few modifications need to be made to the ``CMakeLists.txt`` example above.

.. code-block:: cmake

    ## Set the minimum required version of cmake for your project
    ##   This is not strictly required, however, it is a good practice
    cmake_minimum_required(VERSION 3.20)

    ## Disable in-source builds
    ##   This is not strictly required, however, it is a good practice
    if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
        message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
    endif()

    ## Specify target board
    ##   Projects can target multiple boards.  If this is the case for oyur project, 
    ##   you can specify the board during the CMake configure step:
    ##   $ cmake ../ -DBOARD=XCORE-AI-EXPLORER
    set(BOARD XCORE-AI-EXPLORER)

    ## Optionally specify configuration options
    ##   See the XCore SDK documentation for the set of supported options
    set(MULTITILE_BUILD TRUE)
    set(USE_XUD_HIL TRUE)

    ## Include XMOS platform configuration
    ##   NOTE: This must be done *after* setting the board and configuration options
    include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

    ## Set your project definition
    ##   NOTE: This must be done *after* including the XMOS platform
    project(my_project)

    ## Enable the languages for your project
    enable_language(CXX C ASM)

    ## Set compiler flags as needed
    ##   Your flags may vary but below are some typical flags
    set(APP_COMPILER_FLAGS
        "-Os"
        "-report"
        "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
   )

   ## Specify your sources
   ##   NOTE: Be sure to add XMOS_RTOS_PLATFORM_SOURCES to the list
   set(APP_SOURCES
      "main.c"
      ${XMOS_RTOS_PLATFORM_SOURCES}
   )

   ## Specify your includes
   ##   NOTE: Be sure to add XMOS_RTOS_PLATFORM_INCLUDES to the list
   set(APP_INCLUDES
      ${XMOS_RTOS_PLATFORM_INCLUDES}
   )

   ## Optionally specify compile definitions as needed
   add_compile_definitions(
      DEBUG_PRINT_ENABLE=1
   )

   set(RTOS_TILE_LIST 0 1)
   create_multitile_target(RTOS_TILE_LIST)