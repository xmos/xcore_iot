## XCORE_XS3A only examples
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Bare metal examples
    add_subdirectory(bare-metal/explorer_board)
    add_subdirectory(bare-metal/visual_wake_words)
    add_subdirectory(bare-metal/graphic_equaliser)

    ## FreeRTOS examples
    add_subdirectory(freertos/cifar10)
    add_subdirectory(freertos/device_control)
    add_subdirectory(freertos/dispatcher)
    add_subdirectory(freertos/explorer_board)
    add_subdirectory(freertos/getting_started)
    add_subdirectory(freertos/iot)
    add_subdirectory(freertos/l2_cache)
    add_subdirectory(freertos/usb)
else()
    add_subdirectory(freertos/device_control/host)
endif()
