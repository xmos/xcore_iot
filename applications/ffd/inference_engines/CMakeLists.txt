## Create template intent model runner target
add_library(xcore_sdk_app_ffd_inference_engine_template INTERFACE)
target_sources(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
        template/template_inf_eng.c
        template/template_inf_eng_port.c
        template/template_inf_eng_support.c
)
target_include_directories(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
        api
        template
)
target_link_libraries(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
)
target_compile_definitions(xcore_sdk_app_ffd_inference_engine_template
    INTERFACE
)

## Create an alias
add_library(sdk::app::inference_engine::template ALIAS xcore_sdk_app_ffd_inference_engine_template)
