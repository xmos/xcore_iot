

# Don't try to compile anything unless we're on the right platform
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)

  add_subdirectory(lib_mic_array)

  ## Create SDK aliases for lib_mic_array
  add_library( xcore_sdk_hil_lib_mic_array ALIAS lib_mic_array )
  add_library( sdk::hil::lib_mic_array ALIAS lib_mic_array)

endif()
