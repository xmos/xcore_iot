
## Create library target
add_library(xcore_sdk_rtos_sw_services_fatfs STATIC)
target_sources(xcore_sdk_rtos_sw_services_fatfs
    PUBLIC
        FreeRTOS/diskio.c
        FreeRTOS/ffsystem.c
        FreeRTOS/fs_support.c
        thirdparty/src/ff.c
        thirdparty/src/ffunicode.c
)
target_include_directories(xcore_sdk_rtos_sw_services_fatfs
    PUBLIC
        FreeRTOS
        thirdparty/api
)
target_link_libraries(xcore_sdk_rtos_sw_services_fatfs
    PUBLIC
        sdk::rtos::osal
        sdk::rtos::drivers::qspi_io
        sdk::rtos::FreeRTOS_SMP
)

## Create an alias
add_library(sdk::rtos::sw_services::fatfs ALIAS xcore_sdk_rtos_sw_services_fatfs)
