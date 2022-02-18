
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_lib_random STATIC)
    target_sources(xcore_sdk_lib_random
        PRIVATE
            lib_random/lib_random/src/random.xc
            lib_random/lib_random/src/random_init.c
    )
    target_include_directories(xcore_sdk_lib_random
        PUBLIC
            lib_random/lib_random/api
    )

    ## Create an alias
    add_library(sdk::lib_random ALIAS xcore_sdk_lib_random)
endif()
