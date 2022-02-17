
## Create library target
add_library(xcore_sdk_rtos_sw_services_json INTERFACE)
target_include_directories(xcore_sdk_rtos_sw_services_json
    INTERFACE
        thirdparty/jsmn
)

## Create an alias
add_library(sdk::rtos::sw_services::json ALIAS xcore_sdk_rtos_sw_services_json)
