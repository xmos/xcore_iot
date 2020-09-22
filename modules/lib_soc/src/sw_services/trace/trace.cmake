set(SW_SVC_NAME TRACE)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/trace/FreeRTOS/ASCII/ascii_trace.c")
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/trace/FreeRTOS"
        "src/sw_services/trace/FreeRTOS/ASCII")
set(SW_SVC_DEPENDENT_MODULES
        "")
set(SW_SVC_OPTIONAL_HEADERS
        "xcore_trace_config.h")
