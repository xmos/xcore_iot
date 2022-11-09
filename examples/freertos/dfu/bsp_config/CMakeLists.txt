## Create Explorer Board 2V0 target
add_library(example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0 INTERFACE)
target_sources(example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0
    INTERFACE
        XCORE-AI-EXPLORER_2V0/platform/driver_instances.c
        XCORE-AI-EXPLORER_2V0/platform/platform_init.c
        XCORE-AI-EXPLORER_2V0/platform/platform_start.c
)
target_include_directories(example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0
    INTERFACE
        XCORE-AI-EXPLORER_2V0
)
target_link_libraries(example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0
    INTERFACE
        core::general
        rtos::freertos
        rtos::drivers::general
        rtos::drivers::usb
        rtos::drivers::dfu_image
        rtos::sw_services::usb
        
)
target_compile_definitions(example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0
    INTERFACE
        XCOREAI_EXPLORER=1
        PLATFORM_SUPPORTS_TILE_0=1
        PLATFORM_SUPPORTS_TILE_1=1
        PLATFORM_SUPPORTS_TILE_2=0
        PLATFORM_SUPPORTS_TILE_3=0
        USB_TILE_NO=0
        USB_TILE=tile[USB_TILE_NO]
)

## Create an alias
add_library(example::freertos::dfu::bsp_config::xcore_ai_explorer ALIAS example_freertos_dfu_board_support_config_xcore_ai_explorer_2V0)
