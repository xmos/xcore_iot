
## Create custom board targets for dacs
add_library(xcore_sdk_app_ffd_board_support_dac_aic3204 INTERFACE)
target_sources(xcore_sdk_app_ffd_board_support_dac_aic3204
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/aic3204/aic3204.c
)
target_include_directories(xcore_sdk_app_ffd_board_support_dac_aic3204
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/aic3204
)
target_compile_definitions(xcore_sdk_app_ffd_board_support_dac_aic3204
    INTERFACE
        AIC3204=1
)

## Create an alias
add_library(sdk::app::ffd::dac::aic3204 ALIAS xcore_sdk_app_ffd_board_support_dac_aic3204)

## Create custom board targets for dacs
add_library(xcore_sdk_app_ffd_board_support_dac_dac3103 INTERFACE)
target_sources(xcore_sdk_app_ffd_board_support_dac_dac3103
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/dac3101/dac3101.c
)
target_include_directories(xcore_sdk_app_ffd_board_support_dac_dac3103
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/dac3101
)
target_compile_definitions(xcore_sdk_app_ffd_board_support_dac_dac3103
    INTERFACE
        DAC3101=1
)

## Create an alias
add_library(sdk::app::ffd::dac::dac3101 ALIAS xcore_sdk_app_ffd_board_support_dac_dac3103)
