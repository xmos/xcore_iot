set(SW_SVC_NAME FATFS)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/fatfs/thirdparty/fatfs/src/ff.c"
        "src/sw_services/fatfs/thirdparty/fatfs/src/ffunicode.c"
        "src/sw_services/fatfs/freertos/src/diskio.c"
        "src/sw_services/fatfs/freertos/src/ffsystem.c")
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/fatfs/thirdparty/fatfs/api"
        "src/sw_services/fatfs/freertos/api")
set(SW_SVC_DEPENDENT_MODULES
        "")
set(SW_SVC_OPTIONAL_HEADERS
        "ff_appconf.h")
