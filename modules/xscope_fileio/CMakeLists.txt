add_library(xcore_sdk_modules_xscope_fileio  INTERFACE)

target_sources(xcore_sdk_modules_xscope_fileio
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/xscope_fileio/xscope_fileio/src/xscope_io_device.c)

target_include_directories(xcore_sdk_modules_xscope_fileio
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/xscope_fileio/xscope_fileio
        ${CMAKE_CURRENT_LIST_DIR}/xscope_fileio/xscope_fileio/api)

target_compile_definitions(xcore_sdk_modules_xscope_fileio
    INTERFACE
        TEST_WAV_XSCOPE=1)

add_library(sdk::xscope_fileio ALIAS xcore_sdk_modules_xscope_fileio)
