
## Create library target
add_library(xcore_sdk_legacy_compat INTERFACE)
target_include_directories(xcore_sdk_legacy_compat
    INTERFACE
        api
)
target_link_libraries(xcore_sdk_legacy_compat
    INTERFACE
        sdk::utils
)

## Create an alias
add_library(sdk::legacy_compat ALIAS xcore_sdk_legacy_compat)
