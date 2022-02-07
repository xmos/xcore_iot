
## Create library target
add_library(xcore_sdk_legacy_compat INTERFACE api)

## Create an alias
add_library(sdk::legacy_compat ALIAS xcore_sdk_legacy_compat)
