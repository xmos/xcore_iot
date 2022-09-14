if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create Explorer Board 2V0 target
    add_library(example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0 INTERFACE)
    target_sources(example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCORE-AI-EXPLORER_2V0/platform/driver_instances.c
            XCORE-AI-EXPLORER_2V0/platform/platform_init.c
            XCORE-AI-EXPLORER_2V0/platform/platform_start.c
    )
    target_include_directories(example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCORE-AI-EXPLORER_2V0
    )
    target_link_libraries(example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            core::general
            rtos::freertos
            rtos::drivers::general
    )
    target_compile_definitions(example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCOREAI_EXPLORER=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
    )

    ## Create an alias
    add_library(example::freertos::xlink::bsp_config::xcore_ai_explorer_2V0 ALIAS example_freertos_xlink_board_support_config_xcore_ai_explorer_2V0)
endif()
