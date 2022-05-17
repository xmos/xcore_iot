#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/usb
    ${CMAKE_CURRENT_LIST_DIR}/src/ww_model_runner
)

include(${CMAKE_CURRENT_LIST_DIR}/bsp_config/bsp_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/audio_pipeline/audio_pipeline.cmake)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XUD_CORE_CLOCK=600

    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    CFG_TUSB_DEBUG=0

    ## For USB input and output testing
    # appconfUSB_ENABLED=1
    # appconfAEC_REF_DEFAULT=appconfAEC_REF_USB
    # appconfMIC_SRC_DEFAULT=appconfMIC_SRC_USB
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_COMMON_LINK_LIBRARIES
    sdk::core
    sdk::rtos_freertos
    sdk::rtos::usb_device_control
    sdk::rtos::audio_drivers
    sdk::lib_src
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_application_stlp)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sdk::app::stlp::xk_voice_l71
        sdk::app::stlp::ap::fixeddelay
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_stlp)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME}
    PUBLIC
        ${APP_COMMON_LINK_LIBRARIES}
        sdk::app::stlp::xk_voice_l71
        sdk::app::stlp::ap::fixeddelay
)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(application_stlp tile0_application_stlp tile1_application_stlp 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(application_stlp)
create_debug_target(application_stlp)
create_flash_app_target(application_stlp)

#**********************
# XMOS Development Targets
#**********************
include(${CMAKE_CURRENT_LIST_DIR}/stlp_dev.cmake)

#**********************
# Filesystem support targets
#**********************
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    add_custom_command(
        OUTPUT application_stlp_fat.fs
        COMMAND ${CMAKE_COMMAND} -E make_directory %temp%/fatmktmp/fs
        COMMAND ${CMAKE_COMMAND} -E copy demo.txt %temp%/fatmktmp/fs/demo.txt
        COMMAND fatfs_mkimage --input=%temp%/fatmktmp --output=application_stlp_fat.fs
        BYPRODUCTS %temp%/fatmktmp
        DEPENDS application_stlp
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT application_stlp_fat.fs
        COMMAND bash -c "tmp_dir=$(mktemp -d) && fat_mnt_dir=$tmp_dir && mkdir -p $fat_mnt_dir && mkdir $fat_mnt_dir/fs && cp ./demo.txt $fat_mnt_dir/fs/demo.txt && fatfs_mkimage --input=$tmp_dir --output=application_stlp.fs"
        DEPENDS application_stlp
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
endif()

add_custom_target(flash_fs_application_stlp
    COMMAND xflash --quad-spi-clock 50MHz --factory application_stlp.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/application_stlp_fat.fs
    DEPENDS application_stlp_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)
