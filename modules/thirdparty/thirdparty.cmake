cmake_minimum_required(VERSION 3.20)

#**********************
# Paths
#**********************
set(THIRDPARTY_DIR "$ENV{XCORE_SDK_PATH}/modules/thirdparty")

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
