cmake_minimum_required(VERSION 3.20)

#**********************
# Paths
#**********************

#**********************
# Get path to XCore SDK
#**********************
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

set(THIRDPARTY_DIR "${XCORE_SDK_PATH}/modules/thirdparty")

#**********************
# Set user variables
#**********************
set(THIRDPARTY_SOURCES
    ""
)

set(THIRDPARTY_INCLUDES
    ""
)

list(REMOVE_DUPLICATES THIRDPARTY_SOURCES)
list(REMOVE_DUPLICATES THIRDPARTY_INCLUDES)
