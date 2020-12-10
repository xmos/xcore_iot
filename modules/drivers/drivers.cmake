cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(DRIVERS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers")

#********************************
# Gather various sources
#********************************
include("${DRIVERS_DIR}/hil/hil.cmake")
include("${DRIVERS_DIR}/rtos/rtos.cmake")

#**********************
# set user variables
#**********************
set(DRIVERS_SOURCES
    ${HIL_SOURCES}
    ${RTOS_SOURCES}
)

set(DRIVERS_INCLUDES
    ${HIL_INCLUDES}
    ${RTOS_INCLUDES}
)
