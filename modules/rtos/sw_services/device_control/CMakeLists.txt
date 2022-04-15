
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_device_control INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_device_control
        INTERFACE
            src/device_control.c
            src/resource_table.c
            transport/i2c/device_control_i2c.c
            transport/usb/device_control_usb.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_device_control
        INTERFACE
            api
            transport/i2c
            transport/usb
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_device_control
        INTERFACE
            sdk::rtos::osal
            sdk::rtos::drivers::i2c
            sdk::rtos::drivers::spi
            sdk::rtos::sw_services::usb
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::device_control ALIAS xcore_sdk_rtos_sw_services_device_control)
else()
    ## Host app
    add_library(xcore_sdk_rtos_sw_services_device_control_host_usb INTERFACE)

    # Discern OS for libusb library location
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_directories(xcore_sdk_rtos_sw_services_device_control_host_usb INTERFACE "host/libusb/OSX64")
        set(libusb-1.0_INCLUDE_DIRS "host/libusb/OSX64")
        set(LINK_LIBS usb-1.0.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        find_package(PkgConfig)
        pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
        set(LINK_LIBS usb-1.0)
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_directories(xcore_sdk_rtos_sw_services_device_control_host_usb INTERFACE "host/libusb/Win32")
        set(libusb-1.0_INCLUDE_DIRS "host/libusb/Win32")
        set(LINK_LIBS libusb)
    endif()

    target_sources(xcore_sdk_rtos_sw_services_device_control_host_usb
        INTERFACE
            host/util.c
            host/device_access_usb.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_device_control_host_usb
        INTERFACE
            api
            host
            ${libusb-1.0_INCLUDE_DIRS}
    )
    target_compile_definitions(xcore_sdk_rtos_sw_services_device_control_host_usb INTERFACE USE_USB=1)
    target_link_libraries(xcore_sdk_rtos_sw_services_device_control_host_usb
        INTERFACE
            ${LINK_LIBS}
    )

    ## Create an alias
    add_library(sdk::device_control_host_usb ALIAS xcore_sdk_rtos_sw_services_device_control_host_usb)
endif()
