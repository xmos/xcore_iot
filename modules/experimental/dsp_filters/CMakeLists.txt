
######
add_library(sdk_dsp_filters STATIC)
target_sources(sdk_dsp_filters
    PRIVATE
        src/dsp_filters.c
)
target_include_directories(sdk_dsp_filters
    PUBLIC
        api
)
target_link_libraries(sdk_dsp_filters
    PUBLIC
        sdk::lib_xs3_math
)

add_library(sdk::dsp::filters ALIAS sdk_dsp_filters)
