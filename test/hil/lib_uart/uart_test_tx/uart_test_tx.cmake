#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -target=XCORE-AI-EXPLORER
)
set(APP_LINK_OPTIONS
    -report
    -target=XCORE-AI-EXPLORER
)

#**********************
# Tile Targets
#**********************
add_executable(test_hil_uart_tx_test EXCLUDE_FROM_ALL)
target_sources(test_hil_uart_tx_test PUBLIC ${APP_SOURCES})
target_include_directories(test_hil_uart_tx_test PUBLIC ${APP_INCLUDES})
target_compile_definitions(test_hil_uart_tx_test PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(test_hil_uart_tx_test PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(test_hil_uart_tx_test PUBLIC sdk::hil::lib_uart sdk::utils)
target_link_options(test_hil_uart_tx_test PRIVATE ${APP_LINK_OPTIONS})
set_target_properties(test_hil_uart_tx_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

#=============================================================================

# COMMON_FLAGS = -save-temps -O2 -report $(EXTRA_FLAGS)

# baud ?= 115200
# parity ?= UART_PARITY_NONE

# XCC_FLAGS_smoke = $(COMMON_FLAGS) -DSMOKE_TEST=1 -DBAUD=$(baud)
# XCC_FLAGS_full  = $(COMMON_FLAGS) -DFULL_TEST=1 -DBAUD=$(baud)

# # The USED_MODULES variable lists other module used by the application.

# USED_MODULES = lib_uart lib_xassert lib_logging

# test: bin/app_uart_test.xe
#     $(info Baud: $(baud))
#     xsim bin/app_uart_test.xe --xscope "-offline uart_test.xmt" 
