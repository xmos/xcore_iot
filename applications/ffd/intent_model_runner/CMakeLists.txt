## Create template intent model runner target
add_library(xcore_sdk_app_ffd_intent_model_runner_template INTERFACE)
target_sources(xcore_sdk_app_ffd_intent_model_runner_template
    INTERFACE
        src/intent_model_runner.c
        src/template/model_runner.c
)
target_include_directories(xcore_sdk_app_ffd_intent_model_runner_template
    INTERFACE
        api
)
target_link_libraries(xcore_sdk_app_ffd_intent_model_runner_template
    INTERFACE
)
target_compile_definitions(xcore_sdk_app_ffd_intent_model_runner_template
    INTERFACE
)

## Create an alias
add_library(sdk::app::intent_model_runner::template ALIAS xcore_sdk_app_ffd_intent_model_runner_template)
