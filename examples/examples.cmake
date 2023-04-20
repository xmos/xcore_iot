## XCORE_XS3A only examples
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Bare metal examples
    include(${CMAKE_CURRENT_LIST_DIR}/bare-metal/explorer_board/explorer_board.cmake)

    ## FreeRTOS examples
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/audio_mux/audio_mux.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/dfu/dfu.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/device_control.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/explorer_board/explorer_board.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/getting_started/getting_started.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/iot/iot.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/l2_cache/l2_cache.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/tracealyzer/tracealyzer.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/xlink/xlink.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/xscope_fileio/xscope_fileio.cmake)
else()
    # Determine OS, set up install dir
    if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
        set(HOST_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\bin")
    else()
        set(HOST_INSTALL_DIR "/opt/xmos/bin")
    endif()

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/host)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freertos/tracealyzer/host)
    add_subdirectory(modules/xscope_fileio/xscope_fileio/host)
    install(TARGETS xscope_host_endpoint DESTINATION ${HOST_INSTALL_DIR})
endif()
