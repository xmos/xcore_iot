cmake_minimum_required(VERSION 3.14)

option(DEVICE_CONTROL_USE_USB "Device control host uses USB" ON)
option(DEVICE_CONTROL_USE_RPI_I2C "Device control host uses I2C from a Raspberry Pi " OFF)
option(DEVICE_CONTROL_USE_AARDVARK_I2C "Device control host uses I2C with an Aardvark" OFF)

set(DEVICE_CONTROL_PATH "$ENV{XCORE_SDK_PATH}/modules/rtos/sw_services/device_control/host")

# Tell cmake where to find the macros/functions for things like finding libusb
list(APPEND CMAKE_MODULE_PATH ${DEVICE_CONTROL_PATH}/.cmake)

set(DEVICE_CONTROL_INCLUDES
    "${DEVICE_CONTROL_PATH}"
)

set(DEVICE_CONTROL_SOURCES
    "${DEVICE_CONTROL_PATH}/util.c"
)

set(DEVICE_CONTROL_LIBRARIES "")

if (DEVICE_CONTROL_USE_USB)

    message(STATUS "Building for USB")
    
    add_compile_definitions(USE_USB=1)
    list(APPEND DEVICE_CONTROL_SOURCES "${DEVICE_CONTROL_PATH}/device_access_usb.c")

    # Find libusb, generate error on failure
    find_package(LibUSB)
    if (NOT ${LibUSB_FOUND})
        message(FATAL_ERROR "LibUSB not found.")
    endif ()
    
    list(APPEND DEVICE_CONTROL_LIBRARIES ${LibUSB_LIBRARIES})
    list(APPEND DEVICE_CONTROL_INCLUDES ${LibUSB_INCLUDE_DIRS})
    
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
