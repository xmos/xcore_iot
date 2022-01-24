cmake_minimum_required(VERSION 3.20)

option(DEVICE_CONTROL_USE_USB "Device control host uses USB" ON)
option(DEVICE_CONTROL_USE_RPI_I2C "Device control host uses I2C from a Raspberry Pi " OFF)
option(DEVICE_CONTROL_USE_AARDVARK_I2C "Device control host uses I2C with an Aardvark" OFF)

set(SDK_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET SDK_ROOT_PATH PARENT_PATH SDK_ROOT_PATH)
cmake_path(GET SDK_ROOT_PATH PARENT_PATH SDK_ROOT_PATH)
cmake_path(GET SDK_ROOT_PATH PARENT_PATH SDK_ROOT_PATH)
cmake_path(GET SDK_ROOT_PATH PARENT_PATH SDK_ROOT_PATH)
cmake_path(GET SDK_ROOT_PATH PARENT_PATH SDK_ROOT_PATH)
message(STATUS "Using SDK at ${SDK_ROOT_PATH}")

set(DEVICE_CONTROL_PATH "${SDK_ROOT_PATH}/modules/rtos/sw_services/device_control/host")

# Tell cmake where to find the macros/functions for things like finding libusb
list(APPEND CMAKE_MODULE_PATH ${DEVICE_CONTROL_PATH}/.cmake)

set(DEVICE_CONTROL_INCLUDES
    "${DEVICE_CONTROL_PATH}"
    "${SDK_ROOT_PATH}/modules/rtos/sw_services/device_control/api"
)

set(DEVICE_CONTROL_SOURCES
    "${DEVICE_CONTROL_PATH}/util.c"
)

set(DEVICE_CONTROL_LIBRARIES "")

if (DEVICE_CONTROL_USE_USB)

    message(STATUS "Building for USB")
    
    add_compile_definitions(USE_USB=1)
    list(APPEND DEVICE_CONTROL_SOURCES "${DEVICE_CONTROL_PATH}/device_access_usb.c")

    # Discern OS for libusb library location
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        link_directories("${SDK_ROOT_PATH}/modules/thirdparty/libusb/OSX64")
        set(libusb-1.0_INCLUDE_DIRS "${SDK_ROOT_PATH}/modules/thirdparty/libusb/OSX64")
        set(LINK_LIBS ${LINK_LIBS} usb-1.0.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(PkgConfig)
        pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
        set(LINK_LIBS ${LINK_LIBS} usb-1.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        link_directories("${SDK_ROOT_PATH}/modules/thirdparty/libusb/Win32")
        set(libusb-1.0_INCLUDE_DIRS "${SDK_ROOT_PATH}/modules/thirdparty/libusb/Win32")
        set(LINK_LIBS ${LINK_LIBS} libusb)
    endif()

    list(APPEND DEVICE_CONTROL_LIBRARIES ${LINK_LIBS})
    list(APPEND DEVICE_CONTROL_INCLUDES ${libusb-1.0_INCLUDE_DIRS})

elseif (DEVICE_CONTROL_USE_RPI_I2C)

    message(STATUS "Building for RPi I2C")
    
    add_compile_definitions(USE_I2C=1)
    add_compile_definitions(RPI=1)
    list(APPEND DEVICE_CONTROL_SOURCES "${DEVICE_CONTROL_PATH}/device_access_i2c_rpi.c")
    
elseif (DEVICE_CONTROL_USE_AARDVARK_I2C)

    message(STATUS "Building for Aardvark I2C")
    message(FATAL_ERROR "I2C Aardvark not supported yet")
    
else()

    message(FATAL_ERROR "No device control transport chosen")
    
endif()
