
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_multitile_support INTERFACE)
    target_sources(xcore_sdk_multitile_support
        INTERFACE
            src/soc_intertile_channel.c
            src/tile_map_defaults.c
            src/tile_map.xc
    )
    target_include_directories(xcore_sdk_multitile_support
        INTERFACE
            api
    )

    ## Create an alias
    add_library(sdk::multitile_support ALIAS xcore_sdk_multitile_support)
endif()
