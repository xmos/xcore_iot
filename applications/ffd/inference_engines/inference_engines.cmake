## Create template inference engine target
add_library(xcore_sdk_app_ffd_inference_engine_template INTERFACE)
target_sources(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/template/template_inf_eng.c
        ${CMAKE_CURRENT_LIST_DIR}/template/template_inf_eng_port.c
        ${CMAKE_CURRENT_LIST_DIR}/template/template_inf_eng_support.c
)
target_include_directories(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/template
)
target_link_libraries(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
)
target_compile_definitions(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
)

## Create an alias
add_library(sdk::app::inference_engine::template ALIAS xcore_sdk_app_ffd_inference_engine_template)

## Create keyword inference engine target
add_library(xcore_sdk_app_ffd_inference_engine_keyword INTERFACE)
target_sources(xcore_sdk_app_ffd_inference_engine_keyword
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_features.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_inference.cc
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_inference_port.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_model_data.c
        ${CMAKE_CURRENT_LIST_DIR}/keyword/keyword_model_labels.cc
)
target_include_directories(xcore_sdk_app_ffd_inference_engine_keyword
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/api
        ${CMAKE_CURRENT_LIST_DIR}/keyword
)
target_link_libraries(xcore_sdk_app_ffd_inference_engine_keyword
    INTERFACE
    sdk::inferencing::microfrontend
    sdk::inferencing::rtos
)
target_compile_definitions(xcore_sdk_app_ffd_inference_engine_keyword
    INTERFACE
)

## Create an alias
add_library(sdk::app::inference_engine::keyword ALIAS xcore_sdk_app_ffd_inference_engine_keyword)
