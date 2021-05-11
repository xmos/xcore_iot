include(FetchContent)

FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG        cf949f45ca6d172a177b00da21310607b97bc7a7
  GIT_SHALLOW    TRUE
  SOURCE_DIR     unity
)

FetchContent_GetProperties(unity)
if (NOT unity_POPULATED)
  FetchContent_Populate(unity)
  # Unity has CMake support but not for xcore, so we create manually create variables
  set(UNITY_SOURCES
    PRIVATE "${unity_SOURCE_DIR}/src/unity.c"
    PRIVATE "${unity_SOURCE_DIR}/extras/memory/src/unity_memory.c"
    PRIVATE "${unity_SOURCE_DIR}/extras/fixture/src/unity_fixture.c"
  )
  set(UNITY_INCLUDES
    PRIVATE "${unity_SOURCE_DIR}/src"
    PRIVATE "${unity_SOURCE_DIR}/extras/memory/src"
    PRIVATE "${unity_SOURCE_DIR}/extras/fixture/src"
  )
endif ()
