

add_subdirectory(lib_mic_array)

## Create SDK aliases for lib_mic_array
add_library( xcore_sdk_hil_lib_mic_array ALIAS lib_mic_array )
add_library( sdk::hil::lib_mic_array ALIAS lib_mic_array)