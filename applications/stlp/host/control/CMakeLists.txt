cmake_minimum_required(VERSION 3.14)

project(avona_control LANGUAGES C)
set(TARGET_NAME avona_control)

# Disable in-source build.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

# Tell cmake where to install things
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE PATH "" FORCE)

if (DEVICE_CONTROL_USE_RPI_I2C)
    # Define the cross compiler locations
    set(CMAKE_C_COMPILER   /opt/raspberrypi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER /opt/raspberrypi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
    # Define the sysroot path for the RaspberryPi distribution in our tools folder 
    set(CMAKE_FIND_ROOT_PATH /opt/raspberrypi-tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/)

    # Use our definitions for compiler tools
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    # Search for libraries and headers in the target directories only
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    add_definitions(-std=c11)
endif ()

set(APP_SOURCES
    "avona_control.c"
    "commands.c"
    "argtable/argtable3.c"
)

set(APP_INCLUDES "")

## Import XMOS configurations
if(DEFINED XCORE_SDK_PATH)
    include("${XCORE_SDK_PATH}/modules/rtos/sw_services/device_control/host/device_control_host.cmake")
else()
    include("$ENV{XCORE_SDK_PATH}/modules/rtos/sw_services/device_control/host/device_control_host.cmake")
endif()

add_executable(${TARGET_NAME})

target_link_libraries(${TARGET_NAME} ${DEVICE_CONTROL_LIBRARIES})
target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${DEVICE_CONTROL_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${DEVICE_CONTROL_INCLUDES})

message(STATUS "Building with ${CMAKE_C_COMPILER_ID}")

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	# TODO: Currently building with default compile options
	# These were originally here:
    # target_compile_options(${TARGET_NAME} PRIVATE /O2 /W4)
	
	# These are Windows args used in app_xvf3610/host/dfu_control:
	# target_compile_options(${TARGET_NAME} PUBLIC /RTC1 /nologo /W4 /WX- /O2 /EHa /_CRT_SECURE_NO_WARNINGS)

	target_link_options(${TARGET_NAME} PRIVATE "")
else ()
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
endif ()

