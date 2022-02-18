
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create Explorer Board target
    add_library(xcore_sdk_rtos_board_support_xcore_ai_explorer INTERFACE)
    target_sources(xcore_sdk_rtos_board_support_xcore_ai_explorer
        INTERFACE
            XCORE-AI-EXPLORER/platform/aic3204.c
            XCORE-AI-EXPLORER/platform/app_pll_ctrl.c
            XCORE-AI-EXPLORER/platform/driver_instances.c
            XCORE-AI-EXPLORER/platform/platform_init.c
            XCORE-AI-EXPLORER/platform/platform_start.c
    )
    target_include_directories(xcore_sdk_rtos_board_support_xcore_ai_explorer
        INTERFACE
            XCORE-AI-EXPLORER
    )
    target_link_libraries(xcore_sdk_rtos_board_support_xcore_ai_explorer
        INTERFACE
            sdk::core sdk::rtos_freertos
    )
    target_compile_definitions(xcore_sdk_rtos_board_support_xcore_ai_explorer
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
    add_library(sdk::rtos_bsp::xcore_ai_explorer ALIAS xcore_sdk_rtos_board_support_xcore_ai_explorer)


    ## Create XVF3610_Q60A target
    add_library(xcore_sdk_rtos_board_support_xvf3610_q60a INTERFACE)
    target_compile_definitions(xcore_sdk_rtos_board_support_xvf3610_q60a
        INTERFACE
            XVF3610_Q60A=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
            USB_TILE_NO=0
            USB_TILE=tile[USB_TILE_NO]
    )

    ## Create an alias
    add_library(sdk::rtos_bsp::xvf3610_q60a ALIAS xcore_sdk_rtos_board_support_xvf3610_q60a)



    ## Create Osprey target
    add_library(xcore_sdk_rtos_board_support_osprey INTERFACE)
    target_compile_definitions(xcore_sdk_rtos_board_support_osprey
        INTERFACE
            OSPREY_BOARD=1
            PLATFORM_SUPPORTS_TILE_0=1
            PLATFORM_SUPPORTS_TILE_1=1
            PLATFORM_SUPPORTS_TILE_2=0
            PLATFORM_SUPPORTS_TILE_3=0
            USB_TILE_NO=0
            USB_TILE=tile[USB_TILE_NO]
    )

    ## Create an alias
    add_library(sdk::rtos_bsp::osprey ALIAS xcore_sdk_rtos_board_support_osprey)
endif()


if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A)
    ## Create XS200 Mic array target
    add_library(xcore_sdk_rtos_board_support_xs2_mic_array INTERFACE)
    target_compile_definitions(xcore_sdk_rtos_board_support_xs2_mic_array
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
    add_library(sdk::rtos_bsp::xs200_micarray ALIAS xcore_sdk_rtos_board_support_xs2_mic_array)
endif()
