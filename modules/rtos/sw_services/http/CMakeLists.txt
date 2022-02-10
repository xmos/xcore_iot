
## Create library target
add_library(xcore_sdk_rtos_sw_services_http INTERFACE)
target_sources(xcore_sdk_rtos_sw_services_http
    INTERFACE
        thirdparty/coreHTTP/source/dependency/3rdparty/http_parser/http_parser.c
)
target_include_directories(xcore_sdk_rtos_sw_services_http
    INTERFACE
        thirdparty/coreHTTP/source/dependency/3rdparty/http_parser
)

## Create an alias
add_library(sdk::rtos::sw_services::http ALIAS xcore_sdk_rtos_sw_services_http)
