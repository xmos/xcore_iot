
## Create custom board targets for application
add_library(xcore_sdk_app_ffd_board_support_xk_voice_l71 INTERFACE)
target_sources(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        platform/dac_port.c
        platform/app_pll_ctrl.c
        platform/driver_instances.c
        platform/platform_init.c
        platform/platform_start.c
)
target_include_directories(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        sdk::core
        sdk::rtos_freertos
        sdk::rtos::audio_drivers
        sdk::app::ffd::dac::dac3101
)
target_compile_options(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/XK_VOICE_L71.xn
)
target_link_options(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/XK_VOICE_L71.xn
)
target_compile_definitions(xcore_sdk_app_ffd_board_support_xk_voice_l71
    INTERFACE
        XK_VOICE_L71=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0
        USB_TILE_NO=0
        USB_TILE=tile[USB_TILE_NO]
)

## Create an alias
add_library(sdk::app::ffd::xk_voice_l71 ALIAS xcore_sdk_app_ffd_board_support_xk_voice_l71)
