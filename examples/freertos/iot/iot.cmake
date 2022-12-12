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
    XUD_CORE_CLOCK=600
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
    # Default to Windows PowerShell, if PowerShell Core does not exist.
    find_program(POWERSHELL_EXECUTABLE NAMES pwsh powershell)
    set(NW_PROFILE_SHELL ${POWERSHELL_EXECUTABLE})
    set(NW_PROFILE_SCRIPT "wifi_profile.ps1")
    set(MK_CERTS_SHELL ${POWERSHELL_EXECUTABLE})
    set(MK_CERTS_SCRIPT "make_certs.ps1")
else()
    find_package(Python3 COMPONENTS Interpreter)
    set(NW_PROFILE_SHELL ${Python3_EXECUTABLE})
    set(NW_PROFILE_SCRIPT "wifi_profile.py")
    set(MK_CERTS_SHELL bash)
    set(MK_CERTS_SCRIPT "make_certs.sh")
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/networks.dat
    COMMAND ${NW_PROFILE_SHELL} ${NW_PROFILE_SCRIPT}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    COMMENT "Create wifi profiles file"
    USES_TERMINAL
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/ca.crt
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/ca.key
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/ca.srl
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/client.csr
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/client.crt
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/client.key
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/server.csr
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/server.crt
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/server.key
    COMMAND ${MK_CERTS_SHELL} ${MK_CERTS_SCRIPT}
    BYPRODUCTS
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    COMMENT "Create crypto credential files"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(example_freertos_iot_fat.fs ALL
    COMMAND ${CMAKE_COMMAND} -E copy mqtt_broker_certs/ca.crt fatmktmp/crypto/ca.pem
    COMMAND ${CMAKE_COMMAND} -E copy mqtt_broker_certs/client.crt fatmktmp/crypto/cert.pem
    COMMAND ${CMAKE_COMMAND} -E copy mqtt_broker_certs/client.key fatmktmp/crypto/key.pem
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/modules/rtos/modules/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec fatmktmp/firmware/wf200.sec
    COMMAND ${CMAKE_COMMAND} -E copy networks.dat fatmktmp/wifi/networks.dat
    COMMAND fatfs_mkimage --input=fatmktmp --output=example_freertos_iot_fat.fs
    BYPRODUCTS
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_iot_fat.fs
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/networks.dat
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/ca.crt
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/client.crt
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/mqtt_broker_certs/client.key
    COMMENT
        "Create filesystem"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    VERBATIM
)

set_target_properties(example_freertos_iot_fat.fs PROPERTIES
    ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/fatmktmp
)

create_filesystem_target(example_freertos_iot)
create_flash_app_target(
    #[[ Target ]]                  example_freertos_iot
    #[[ Boot Partition Size ]]     0x100000
    #[[ Data Partition Contents ]] example_freertos_iot_fat.fs
    #[[ Dependencies ]]            make_fs_example_freertos_iot
)
