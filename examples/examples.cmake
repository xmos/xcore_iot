## XCORE_XS3A only examples
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Bare metal examples
    include(${CMAKE_CURRENT_LIST_DIR}/bare-metal/explorer_board/explorer_board.cmake)

    ## FreeRTOS examples
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/dfu/dfu.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/device_control.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/dispatcher/dispatcher.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/explorer_board/explorer_board.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/getting_started/getting_started.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/iot/iot.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/l2_cache/l2_cache.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/tracealyzer/tracealyzer.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/usb/usb.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/xlink/xlink.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/xscope_fileio/xscope_fileio.cmake)
else()
    # Get the "version" value from the JSON element
    file(READ settings.json JSON_STRING)
    string(JSON SDK_VERSION GET ${JSON_STRING} ${IDX} version)

    # Determine OS, set up install dir
    if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
        set(HOST_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\SDK\\${SDK_VERSION}\\bin")
    else()
        set(HOST_INSTALL_DIR "/opt/xmos/SDK/${SDK_VERSION}/bin")
    endif()

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/host)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freertos/tracealyzer/host)
    add_subdirectory(modules/xscope_fileio/xscope_fileio/host)
    install(TARGETS xscope_host_endpoint DESTINATION ${HOST_INSTALL_DIR})
endif()
