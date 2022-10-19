
## Add frameworks
add_subdirectory(core)
add_subdirectory(io)
add_subdirectory(rtos)
add_subdirectory(voice)

## Add additional modules
add_subdirectory(sample_rate_conversion)
add_subdirectory(xscope_fileio)

## The following alias is added to support in intermediate version of fwk_voick
##  This can be removed once fwk_voice is updated to use the new, core::lib_tflite_micro alias
add_library(sdk::inferencing::lib_tflite_micro ALIAS inferencing_tflite_micro)

