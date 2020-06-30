set(SW_SVC_NAME LIB_SOC_SW_TLS_SUPPORT)
set(SW_SVC_ADD_COMPILER_FLAGS "")
set(SW_SVC_XC_SRCS
        "")
set(SW_SVC_C_SRCS
        "src/sw_services/tls_support/freertos/src/mbedtls_support.c"
        )
set(SW_SVC_ASM_SRCS
        "")
set(SW_SVC_INCLUDES
        "src/sw_services/tls_support/api"
        "src/sw_services/tls_support/freertos/includes")
set(SW_SVC_DEPENDENT_MODULES
        "mbedtls(>=2.16.6)"
        )
set(SW_SVC_OPTIONAL_HEADERS
        "")
