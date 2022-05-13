#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

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
    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    CFG_TUSB_DEBUG=0

    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    USB_TILE_NO=0
    USB_TILE=tile[USB_TILE_NO]
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
    sdk::core
    sdk::rtos::audio_drivers
    sdk::rtos_freertos
    sdk::rtos::usb_device_control
    sdk::rtos_bsp::xcore_ai_explorer
)

# **********************
# Audio Test Tile Targets
# **********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/audio_test/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/audio_test/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_audio_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_audio_test tile0_example_freertos_usb_tusb_demo_audio_test tile1_example_freertos_usb_tusb_demo_audio_test 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_audio_test)
create_debug_target(example_freertos_usb_tusb_demo_audio_test)


#**********************
# CDC Dual Ports Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/cdc_dual_ports/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/cdc_dual_ports/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_cdc_dual_ports)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_cdc_dual_ports)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_cdc_dual_ports tile0_example_freertos_usb_tusb_demo_cdc_dual_ports tile1_example_freertos_usb_tusb_demo_cdc_dual_ports 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_cdc_dual_ports)
create_debug_target(example_freertos_usb_tusb_demo_cdc_dual_ports)


#**********************
# CDC MSC Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/cdc_msc/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/cdc_msc/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_cdc_msc)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_cdc_msc)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_cdc_msc tile0_example_freertos_usb_tusb_demo_cdc_msc tile1_example_freertos_usb_tusb_demo_cdc_msc 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_cdc_msc)
create_debug_target(example_freertos_usb_tusb_demo_cdc_msc)


#**********************
# DFU Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/dfu/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/dfu/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED
                               DFU_DEMO=1
)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_dfu)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_dfu)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_dfu tile0_example_freertos_usb_tusb_demo_dfu tile1_example_freertos_usb_tusb_demo_dfu 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_dfu)
create_debug_target(example_freertos_usb_tusb_demo_dfu)


# Incomplete pending xcore tools updates
#**********************
# DFU Runtime Tile Targets
#**********************
# file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/dfu_runtime/src/*.c )
# set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/dfu_runtime/src/)
# set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED
#                                DFU_DEMO=1
# )
# set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_dfu_runtime)
# add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
# target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
# target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
# target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
# target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
# target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
# target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
# unset(TARGET_NAME)
#
# set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_dfu_runtime)
# add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
# target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
# target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
# target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
# target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
# target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
# target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
# unset(TARGET_NAME)
# unset(DEMO_SOURCES)
# unset(DEMO_INCLUDES)
# unset(DEMO_COMPILE_DEFINITIONS)
#
# #**********************
# # Merge binaries
# #**********************
# merge_binaries(example_freertos_usb_tusb_demo_dfu_runtime tile0_example_freertos_usb_tusb_demo_dfu_runtime tile1_example_freertos_usb_tusb_demo_dfu_runtime 1)
#
# #**********************
# # Create run and debug targets
# #**********************
# create_run_target(example_freertos_usb_tusb_demo_dfu_runtime)
# create_debug_target(example_freertos_usb_tusb_demo_dfu_runtime)


#**********************
# HID Boot Inferface Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_boot_interface/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_boot_interface/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_hid_boot_interface)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_hid_boot_interface)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_hid_boot_interface tile0_example_freertos_usb_tusb_demo_hid_boot_interface tile1_example_freertos_usb_tusb_demo_hid_boot_interface 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_hid_boot_interface)
create_debug_target(example_freertos_usb_tusb_demo_hid_boot_interface)


#**********************
# HID Composite Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_composite/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_composite/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_hid_composite)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_hid_composite)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_hid_composite tile0_example_freertos_usb_tusb_demo_hid_composite tile1_example_freertos_usb_tusb_demo_hid_composite 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_hid_composite)
create_debug_target(example_freertos_usb_tusb_demo_hid_composite)


#**********************
# HID Generic In/Out Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_generic_inout/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_generic_inout/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_hid_generic_inout)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_hid_generic_inout)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_hid_generic_inout tile0_example_freertos_usb_tusb_demo_hid_generic_inout tile1_example_freertos_usb_tusb_demo_hid_generic_inout 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_hid_generic_inout)
create_debug_target(example_freertos_usb_tusb_demo_hid_generic_inout)


#**********************
# HID Multiple Interface Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_multiple_interface/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/hid_multiple_interface/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_hid_multiple_interface)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_hid_multiple_interface)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_hid_multiple_interface tile0_example_freertos_usb_tusb_demo_hid_multiple_interface tile1_example_freertos_usb_tusb_demo_hid_multiple_interface 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_hid_multiple_interface)
create_debug_target(example_freertos_usb_tusb_demo_hid_multiple_interface)


#**********************
# MIDI Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/midi_test/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/midi_test/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_midi_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_midi_test)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_midi_test tile0_example_freertos_usb_tusb_demo_midi_test tile1_example_freertos_usb_tusb_demo_midi_test 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_midi_test)
create_debug_target(example_freertos_usb_tusb_demo_midi_test)


#**********************
# MSC Dual Disk Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/msc_dual_lun/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/msc_dual_lun/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_msc_dual_lun)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_msc_dual_lun)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_msc_dual_lun tile0_example_freertos_usb_tusb_demo_msc_dual_lun tile1_example_freertos_usb_tusb_demo_msc_dual_lun 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_msc_dual_lun)
create_debug_target(example_freertos_usb_tusb_demo_msc_dual_lun)


#**********************
# UAC2 Headset Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/uac2_headset/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/uac2_headset/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_uac2_headset)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_uac2_headset)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_uac2_headset tile0_example_freertos_usb_tusb_demo_uac2_headset tile1_example_freertos_usb_tusb_demo_uac2_headset 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_uac2_headset)
create_debug_target(example_freertos_usb_tusb_demo_uac2_headset)


#**********************
# USBTMC Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/usbtmc/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/usbtmc/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_FULL_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_usbtmc)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_usbtmc)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_usbtmc tile0_example_freertos_usb_tusb_demo_usbtmc tile1_example_freertos_usb_tusb_demo_usbtmc 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_usbtmc)
create_debug_target(example_freertos_usb_tusb_demo_usbtmc)


#**********************
# Video Capture Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/video_capture/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/video_capture/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_FULL_SPEED CFG_EXAMPLE_VIDEO_READONLY=1)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_video_capture)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_video_capture)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_video_capture tile0_example_freertos_usb_tusb_demo_video_capture tile1_example_freertos_usb_tusb_demo_video_capture 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_video_capture)
create_debug_target(example_freertos_usb_tusb_demo_video_capture)


#**********************
# Webusb serial Tile Targets
#**********************
file(GLOB_RECURSE DEMO_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/webusb_serial/src/*.c )
set(DEMO_INCLUDES              ${CMAKE_CURRENT_LIST_DIR}/tinyusb_demos/webusb_serial/src/)
set(DEMO_COMPILE_DEFINITIONS   BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
set(TARGET_NAME tile0_example_freertos_usb_tusb_demo_webusb_serial)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_usb_tusb_demo_webusb_serial)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES} ${DEMO_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES} ${DEMO_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} ${DEMO_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)
unset(DEMO_SOURCES)
unset(DEMO_INCLUDES)
unset(DEMO_COMPILE_DEFINITIONS)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_usb_tusb_demo_webusb_serial tile0_example_freertos_usb_tusb_demo_webusb_serial tile1_example_freertos_usb_tusb_demo_webusb_serial 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_usb_tusb_demo_webusb_serial)
create_debug_target(example_freertos_usb_tusb_demo_webusb_serial)
