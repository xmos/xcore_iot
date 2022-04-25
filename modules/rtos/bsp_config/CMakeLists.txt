
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create Explorer Board 1V1 target
    add_library(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1 INTERFACE)
    target_sources(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1
        INTERFACE
            XCORE-AI-EXPLORER_1V1/platform/aic3204.c
            XCORE-AI-EXPLORER_1V1/platform/app_pll_ctrl.c
            XCORE-AI-EXPLORER_1V1/platform/driver_instances.c
            XCORE-AI-EXPLORER_1V1/platform/platform_init.c
            XCORE-AI-EXPLORER_1V1/platform/platform_start.c
    )
    target_include_directories(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1
        INTERFACE
            XCORE-AI-EXPLORER_1V1
    )
    target_link_libraries(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1
        INTERFACE
            sdk::core
            sdk::rtos_freertos
            sdk::rtos::audio_drivers
    )
    target_compile_definitions(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1
        INTERFACE
            XCOREAI_EXPLORER=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
            USB_TILE_NO=0
            USB_TILE=tile[USB_TILE_NO]

            MIC_ARRAY_CONFIG_MCLK_FREQ=24576000
            MIC_ARRAY_CONFIG_PDM_FREQ=3072000
            MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=256
            MIC_ARRAY_CONFIG_MIC_COUNT=2
            MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
            MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
            MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN
            MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
            MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
            # DEBUG_PRINT_ENABLE_RTOS_MIC_ARRAY=1
    )

    ## Create an alias
    add_library(sdk::rtos_bsp::xcore_ai_explorer_1V1 ALIAS xcore_sdk_rtos_board_support_config_xcore_ai_explorer_1V1)


    ## Create Explorer Board 1V0 target
    add_library(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0 INTERFACE)
    target_sources(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCORE-AI-EXPLORER_2V0/platform/aic3204.c
            XCORE-AI-EXPLORER_2V0/platform/app_pll_ctrl.c
            XCORE-AI-EXPLORER_2V0/platform/driver_instances.c
            XCORE-AI-EXPLORER_2V0/platform/platform_init.c
            XCORE-AI-EXPLORER_2V0/platform/platform_start.c
    )
    target_include_directories(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCORE-AI-EXPLORER_2V0
    )
    target_link_libraries(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            sdk::core
            sdk::rtos_freertos
            sdk::rtos::audio_drivers
    )
    target_compile_definitions(xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0
        INTERFACE
            XCOREAI_EXPLORER=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
            USB_TILE_NO=0
            USB_TILE=tile[USB_TILE_NO]

            MIC_ARRAY_CONFIG_MCLK_FREQ=24576000
            MIC_ARRAY_CONFIG_PDM_FREQ=3072000
            MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=256
            MIC_ARRAY_CONFIG_MIC_COUNT=2
            MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
            MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
            MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN
            MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
            MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
            # DEBUG_PRINT_ENABLE_RTOS_MIC_ARRAY=1
    )

    ## Create an alias
    add_library(sdk::rtos_bsp::xcore_ai_explorer_2v0 ALIAS xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0)

    ## Create an alias for the DEFAULT explorer board, 2V0
    add_library(sdk::rtos_bsp::xcore_ai_explorer ALIAS xcore_sdk_rtos_board_support_config_xcore_ai_explorer_2V0)
endif()


if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A)
    ## Create XS200 Mic array target
    add_library(xcore_sdk_rtos_board_support_config_xs2_mic_array INTERFACE)
    target_compile_definitions(xcore_sdk_rtos_board_support_config_xs2_mic_array
        INTERFACE
            XCORE200_MIC_ARRAY=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
            USB_TILE_NO=1
            USB_TILE=tile[USB_TILE_NO]
            XUD_SERIES_SUPPORT=XUD_X200_SERIES
    )

    ## Create an alias
    add_library(sdk::rtos_bsp::xs200_micarray ALIAS xcore_sdk_rtos_board_support_config_xs2_mic_array)
endif()
