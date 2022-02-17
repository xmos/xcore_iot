
add_subdirectory(aif)
add_subdirectory(device_memory_support)
add_subdirectory(hil)
add_subdirectory(legacy_compat)
add_subdirectory(lib_dsp)
add_subdirectory(lib_otpinfo)
add_subdirectory(lib_random)
add_subdirectory(lib_src)
add_subdirectory(lib_xs3_math)
add_subdirectory(multitile_support)
add_subdirectory(rtos)
add_subdirectory(thirdparty)
add_subdirectory(utils)


## Create an aggregate of core libraries
add_library(xcore_sdk_core INTERFACE)
target_link_libraries(xcore_sdk_core
    INTERFACE
        sdk::hil
        sdk::lib_dsp
        sdk::legacy_compat
        sdk::lib_xs3_math
        sdk::multitile_support
        sdk::utils
)

add_library(sdk::core ALIAS xcore_sdk_core)

## Create an aggregate of iot libraries
add_library(xcore_sdk_iot INTERFACE)
target_link_libraries(xcore_sdk_iot
    INTERFACE
        sdk::rtos_freertos
        sdk::rtos::wifi
        sdk::rtos::sw_services::http
        sdk::rtos::sw_services::json
        sdk::rtos::sw_services::mqtt
        sdk::rtos::sw_services::sntpd
        sdk::rtos::sw_services::tls_support
)

add_library(sdk::iot ALIAS xcore_sdk_iot)

## Create an aggregate of rtos usb libraries
add_library(xcore_sdk_rtos_usb INTERFACE)
target_link_libraries(xcore_sdk_rtos_usb
    INTERFACE
        sdk::rtos_freertos
        sdk::rtos::sw_services::usb
)

add_library(sdk::rtos_usb ALIAS xcore_sdk_rtos_usb)
