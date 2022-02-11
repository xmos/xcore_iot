
## Create library target
add_library(xcore_sdk_lib_otpinfo STATIC)
target_sources(xcore_sdk_lib_otpinfo
    PUBLIC
        lib_otpinfo/lib_otpinfo/src/otp_board_info.xc
)
target_include_directories(xcore_sdk_lib_otpinfo
    PUBLIC
        lib_otpinfo/lib_otpinfo/api
)

## Create an alias
add_library(sdk::lib_otpinfo ALIAS xcore_sdk_lib_otpinfo)
