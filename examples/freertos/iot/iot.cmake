#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/network_demos
    ${CMAKE_CURRENT_LIST_DIR}/src/mqtt_demo
    ${CMAKE_CURRENT_LIST_DIR}/src/mem_analysis
)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -lquadspi
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    DEBUG_PRINT_ENABLE_IOT_WIFI=1
    DEBUG_PRINT_ENABLE_WIFI_CONN_MGR=1
    DEBUG_PRINT_ENABLE_MQTT_DEMO_CLIENT=1
    DEBUG_PRINT_ENABLE_LIB_SOC_SW_WIFI=1
    MQTTCLIENT_PLATFORM_HEADER=MQTTFreeRTOS.h
    MBEDTLS_CONFIG_FILE=\"mbedtls_sample_config.h\"
    SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR\)
    MQTT_TASK=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XE_BASE_TILE=0
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    core::general
    rtos::drivers::wifi
    rtos::iot
    rtos::bsp_config::xcore_ai_explorer
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_iot)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_iot)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_iot tile0_example_freertos_iot tile1_example_freertos_iot 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_iot)
create_debug_target(example_freertos_iot)
create_install_target(example_freertos_iot)

#**********************
# Filesystem support targets
#**********************
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    message(STATUS "examples/freertos/example_freertos_iot is only available for Linux and Mac")
else()
    find_package( Python3 COMPONENTS Interpreter )

    add_custom_command(
        OUTPUT networks.dat
        COMMAND ${Python3_EXECUTABLE} wifi_profile.py
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        BYPRODUCTS
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/networks.dat
        COMMENT "Create wifi profiles file"
        USES_TERMINAL
        VERBATIM
    )

    add_custom_command(
        OUTPUT mqtt_broker_certs
        COMMAND bash make_certs.sh
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        BYPRODUCTS
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs
        COMMENT "Create crypto credential files"
        USES_TERMINAL
        VERBATIM
    )

    add_custom_command(
        OUTPUT example_freertos_iot_fat.fs
        COMMAND bash -c "tmp_dir=$(mktemp -d) && fat_mnt_dir=$tmp_dir && mkdir -p $fat_mnt_dir && mkdir $fat_mnt_dir/firmware && mkdir $fat_mnt_dir/crypto && mkdir $fat_mnt_dir/wifi && cp mqtt_broker_certs/ca.crt $fat_mnt_dir/crypto/ca.pem && cp mqtt_broker_certs/client.crt $fat_mnt_dir/crypto/cert.pem && cp mqtt_broker_certs/client.key $fat_mnt_dir/crypto/key.pem && cp ${CMAKE_SOURCE_DIR}/modules/rtos/modules/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec $fat_mnt_dir/firmware/wf200.sec && cp networks.dat $fat_mnt_dir/wifi/networks.dat && fatfs_mkimage --input=$tmp_dir --output=example_freertos_iot_fat.fs"
        DEPENDS
            example_freertos_iot
            networks.dat
            mqtt_broker_certs
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
endif()

add_custom_target(flash_fs_example_freertos_iot
    COMMAND xflash --quad-spi-clock 50MHz --factory example_freertos_iot.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_iot_fat.fs
    DEPENDS example_freertos_iot_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)
