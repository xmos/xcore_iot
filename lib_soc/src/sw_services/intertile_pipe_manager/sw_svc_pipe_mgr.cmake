set(SW_SVC_NAME PIPE_MGR)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/intertile_pipe_manager/src/intertile_pipe.c"
        "src/sw_services/intertile_pipe_manager/src/intertile_pipe_buffers.c"
        "src/sw_services/intertile_pipe_manager/src/intertile_pipe_mgr.c")
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/intertile_pipe_manager/api"
        "src/sw_services/intertile_pipe_manager/src")
set(SW_SVC_DEPENDENT_MODULES
        "")
set(SW_SVC_OPTIONAL_HEADERS
        "soc_conf.h")
