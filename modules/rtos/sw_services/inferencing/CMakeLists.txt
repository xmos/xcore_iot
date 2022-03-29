SET(LIB_TFLITE_MICRO_PATH ${CMAKE_SOURCE_DIR}/modules/inferencing/lib_tflite_micro/lib_tflite_micro)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## ******************************
    ## Create inferencing RTOS target
    ## ******************************
    add_library(xcore_sdk_inferencing_rtos INTERFACE)

    ## Source files
    target_sources(xcore_sdk_inferencing_rtos
        INTERFACE
            src/debug_log.cc
            src/xcore_rtos_conv2d_v2.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_custom_options.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_error_reporter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_interpreter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_profiler.cc
    )

    target_include_directories(xcore_sdk_inferencing_rtos
        INTERFACE
            api
            ${LIB_TFLITE_MICRO_PATH}/api
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels
    )
    target_link_libraries(xcore_sdk_inferencing_rtos
        INTERFACE
            sdk::inferencing::lib_nn
            sdk::inferencing::tflite_micro
    )

    ## Create an alias
    add_library(sdk::inferencing::rtos ALIAS xcore_sdk_inferencing_rtos)
endif()
