
# Note, the inclusion of the vanilla API stuff below is a temporary hack until
# a more RTOS-friendly API is available for the mic array
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_mic_array INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_mic_array
        INTERFACE
            src/rtos_mic_array.c
            src/rtos_mic_array_rpc.c
            ../../../hil/lib_mic_array/lib_mic_array/etc/vanilla/mic_array_vanilla.cpp
    )
    target_include_directories(xcore_sdk_rtos_drivers_mic_array
        INTERFACE
            api
            ../../../hil/lib_mic_array/lib_mic_array/etc/vanilla/
    )
    target_link_libraries(xcore_sdk_rtos_drivers_mic_array
        INTERFACE
            sdk::hil::lib_mic_array
            sdk::rtos::osal
    )
    target_compile_definitions(xcore_sdk_rtos_drivers_mic_array
        INTERFACE
            MIC_ARRAY_BASIC_API_ENABLE=1
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::mic_array ALIAS xcore_sdk_rtos_drivers_mic_array)
endif()
