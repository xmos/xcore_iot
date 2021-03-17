if(CMAKE_XC_COMPILER_FORCED)
  # The compiler configuration was forced by the user.
  # Assume the user has configured all compiler information.
  set(CMAKE_XC_COMPILER_WORKS TRUE CACHE INTERNAL "")
  return()
endif()
