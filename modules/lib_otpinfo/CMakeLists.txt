
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_lib_otpinfo STATIC)
    target_sources(xcore_sdk_lib_otpinfo
        PRIVATE
            lib_otpinfo/lib_otpinfo/src/otp_board_info.xc
    )
    target_include_directories(xcore_sdk_lib_otpinfo
        PUBLIC
            lib_otpinfo/lib_otpinfo/api
    )

    ## Create an alias
    add_library(sdk::lib_otpinfo ALIAS xcore_sdk_lib_otpinfo)
endif()
