#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   src/*.c src/*.cc)
set(APP_INCLUDES src src/model_runner)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    TF_LITE_STATIC_MEMORY=1
    XCORE=1
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME cifar10_sram_tile0)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME cifar10_sram_tile1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME cifar10_extmem_tile0)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 USE_EXTMEM=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME cifar10_extmem_tile1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 USE_EXTMEM=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME cifar10_swmem_tile0)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 USE_SWMEM=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME cifar10_swmem_tile1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 USE_SWMEM=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::core sdk::rtos_freertos sdk::aif_model_runner_freertos)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(cifar10_sram cifar10_sram_tile0 cifar10_sram_tile1 1)
merge_binaries(cifar10_extmem cifar10_extmem_tile0 cifar10_extmem_tile1 1)
merge_binaries(cifar10_swmem cifar10_swmem_tile0 cifar10_swmem_tile1 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(cifar10_sram)
create_debug_target(cifar10_sram)
create_run_target(cifar10_extmem)
create_debug_target(cifar10_extmem)
create_run_target(cifar10_swmem)
create_debug_target(cifar10_swmem)

#**********************
# Extract swmem
#**********************
add_custom_command(
    TARGET cifar10_swmem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory cifar10_swmem_split
    COMMAND xobjdump --strip cifar10_swmem.xe
    COMMAND xobjdump --split --split-dir cifar10_swmem_split cifar10_swmem.xb
    BYPRODUCTS
        cifar10_swmem.xb
        cifar10_swmem_split
    VERBATIM
)

#**********************
# Filesystem support targets
#**********************
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    add_custom_command(
        OUTPUT cifar10_fat.fs
        COMMAND echo "Not yet implemented"
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT cifar10_fat.fs
        COMMAND bash -c "tmp_dir=$(mktemp -d) && fat_mnt_dir=$tmp_dir && mkdir -p $fat_mnt_dir && mkdir $fat_mnt_dir/fs && cp ./test_inputs/airplane.bin $fat_mnt_dir/airplane.bin && cp ./test_inputs/bird.bin $fat_mnt_dir/bird.bin && cp ./test_inputs/cat.bin $fat_mnt_dir/cat.bin && cp ./test_inputs/deer.bin $fat_mnt_dir/deer.bin && cp ./test_inputs/frog.bin $fat_mnt_dir/frog.bin && cp ./test_inputs/horse.bin $fat_mnt_dir/horse.bin && cp ./test_inputs/truck.bin $fat_mnt_dir/truck.bin && fatfs_mkimage --input=$tmp_dir --output=cifar10_fat.fs"
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
endif()

add_custom_target(flash_fs_cifar10_sram
    COMMAND xflash --quad-spi-clock 50MHz --factory cifar10_sram.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/cifar10_fat.fs
    DEPENDS cifar10_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)

add_custom_target(flash_fs_cifar10_extmem
    COMMAND xflash --quad-spi-clock 50MHz --factory cifar10_extmem.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/cifar10_fat.fs
    DEPENDS cifar10_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    add_custom_target(flash_fs_cifar10_swmem
        COMMAND echo "Not yet implemented"
        DEPENDS cifar10_fat.fs cifar10_swmem
        COMMENT
            "Flash filesystem and swmem"
        VERBATIM
    )
else()
    add_custom_target(flash_fs_cifar10_swmem
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cifar10_swmem_split/image_n0c1.swmem filesystem_support/flash_image.bin
        COMMAND cat filesystem_support/cifar10_fat.fs | dd of=filesystem_support/flash_image.bin bs=1 seek=1048576 conv=notrunc
        COMMAND xflash --write-all filesystem_support/flash_image.bin --target-file XCORE-AI-EXPLORER.xn
        DEPENDS cifar10_fat.fs cifar10_swmem
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}
        COMMENT
            "Flash filesystem and swmem"
        VERBATIM
    )
endif()
