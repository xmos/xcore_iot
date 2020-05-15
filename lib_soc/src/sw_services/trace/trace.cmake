set(SW_SVC_NAME LIB_SOC_SW_TRACE)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/trace/FreeRTOS/ASCII/ascii_trace.c"
        "src/sw_services/trace/FreeRTOS/SystemView/SEGGER_SYSVIEW.c"
        "src/sw_services/trace/FreeRTOS/SystemView/SEGGER_SYSVIEW_FreeRTOS.c"
        "src/sw_services/trace/FreeRTOS/SystemView/Config/SEGGER_SYSVIEW_Config_FreeRTOS.c")
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/trace/FreeRTOS"
        "src/sw_services/trace/FreeRTOS/ASCII"
        "src/sw_services/trace/FreeRTOS/SystemView"
        "src/sw_services/trace/FreeRTOS/SystemView/Config")
set(SW_SVC_DEPENDENT_MODULES
        "")
set(SW_SVC_OPTIONAL_HEADERS
        "xcore_trace_config.h")
