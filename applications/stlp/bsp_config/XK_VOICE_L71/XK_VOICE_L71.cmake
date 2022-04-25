
## Create custom board targets for application
add_library(xcore_sdk_app_stlp_board_support_xk_voice_l71 INTERFACE)
target_sources(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/platform/dac_port.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/app_pll_ctrl.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/driver_instances.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_init.c
        ${CMAKE_CURRENT_LIST_DIR}/platform/platform_start.c
)
target_include_directories(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        sdk::core
        sdk::rtos_freertos
        sdk::rtos::audio_drivers
        sdk::app::stlp::dac::dac3101
)
target_compile_options(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_link_options(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/XK_VOICE_L71.xn
)
target_compile_definitions(xcore_sdk_app_stlp_board_support_xk_voice_l71
    INTERFACE
        XK_VOICE_L71=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0
        USB_TILE_NO=0
        USB_TILE=tile[USB_TILE_NO]

        MIC_ARRAY_CONFIG_MCLK_FREQ=24576000
        MIC_ARRAY_CONFIG_PDM_FREQ=3072000
        MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=240
        MIC_ARRAY_CONFIG_MIC_COUNT=2
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
        MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
        MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN_OUT
        MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
        MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
)

## Create an alias
add_library(sdk::app::stlp::xk_voice_l71 ALIAS xcore_sdk_app_stlp_board_support_xk_voice_l71)
