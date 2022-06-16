#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cc
)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/image_classifier
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
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XUD_CORE_CLOCK=600
    XCORE=1
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    core::general
    rtos::freertos
    rtos::sw_services::inferencing
    rtos::bsp_config::xcore_ai_explorer
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_cifar10)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_cifar10)
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
merge_binaries(example_freertos_cifar10 tile0_example_freertos_cifar10 tile1_example_freertos_cifar10 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_cifar10)
create_debug_target(example_freertos_cifar10)

#**********************
# Filesystem support targets
#**********************
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    add_custom_command(
        OUTPUT example_freertos_cifar10_fat.fs
        COMMAND ${CMAKE_COMMAND} -E make_directory %temp%/fatmktmp
        COMMAND ${CMAKE_COMMAND} -E copy model.bin %temp%/fatmktmp/model.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/airplane.bin %temp%/fatmktmp/plane.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/automobile.bin %temp%/fatmktmp/auto.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/bird.bin %temp%/fatmktmp/bird.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/cat.bin %temp%/fatmktmp/cat.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/deer.bin %temp%/fatmktmp/deer.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/dog.bin %temp%/fatmktmp/dog.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/frog.bin %temp%/fatmktmp/frog.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/horse.bin %temp%/fatmktmp/horse.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/ship.bin %temp%/fatmktmp/ship.bin
        COMMAND ${CMAKE_COMMAND} -E copy test_inputs/truck.bin %temp%/fatmktmp/truck.bin
        COMMAND fatfs_mkimage --input=%temp%/fatmktmp --output=example_freertos_cifar10_fat.fs
        BYPRODUCTS %temp%/fatmktmp
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT example_freertos_cifar10_fat.fs
        COMMAND bash -c "tmp_dir=$(mktemp -d) && fat_mnt_dir=$tmp_dir && mkdir -p $fat_mnt_dir && cp model.bin $fat_mnt_dir/model.bin && cp test_inputs/airplane.bin $fat_mnt_dir/plane.bin && cp test_inputs/automobile.bin $fat_mnt_dir/auto.bin && cp test_inputs/bird.bin $fat_mnt_dir/bird.bin && cp test_inputs/cat.bin $fat_mnt_dir/cat.bin && cp test_inputs/deer.bin $fat_mnt_dir/deer.bin && cp test_inputs/dog.bin $fat_mnt_dir/dog.bin && cp test_inputs/frog.bin $fat_mnt_dir/frog.bin && cp test_inputs/horse.bin $fat_mnt_dir/horse.bin && cp test_inputs/ship.bin $fat_mnt_dir/ship.bin && cp test_inputs/truck.bin $fat_mnt_dir/truck.bin && fatfs_mkimage --input=$fat_mnt_dir --output=example_freertos_cifar10_fat.fs"
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
endif()

create_filesystem_target(example_freertos_cifar10)

add_custom_target(flash_fs_example_freertos_cifar10
    COMMAND xflash --quad-spi-clock 50MHz --factory example_freertos_cifar10.xe --boot-partition-size 0x100000 --data example_freertos_cifar10_fat.fs
    DEPENDS make_fs_example_freertos_cifar10
    COMMENT
        "Flash filesystem"
    VERBATIM
)
